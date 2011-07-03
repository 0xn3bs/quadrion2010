#include "hdrpipeline.h"



CHDRPipeline::CHDRPipeline()
{
	m_isInitialized = false;
	m_deltaTime = 0.0;
	
	m_depthTarget = QRENDER_DEFAULT;
	m_bUsingLogLuv = false;
	
	m_middleGrey = 0.0F;
	m_bloomScale = 0.0F;
	m_brightnessThreshold = 0.0F;
	m_brightnessOffset = 0.0F;
	m_adaptationFactor = 30.0F;
	m_finalGrey = 0.5F;
	Initialize();
}

CHDRPipeline::~CHDRPipeline()
{
	Destroy();
}





/////////////////////////////////////////////////////////////
// initHDRPipeline
// Must be called prior to using the HDR pipeline to render
// Loads textures, effect, and rendertargets
BOOL CHDRPipeline::Initialize()
{
	// do not re-initialize //
	if( m_isInitialized ) 
		return FALSE;

	// obtain a luminance format //
	SQuadrionDeviceCapabilities* caps = (SQuadrionDeviceCapabilities*)g_pRender->GetDeviceCapabilities();
	m_luminanceFormat = caps->greatestLuminanceFormat;
	
	// keep crops divisible by 8 //
	float fbWidth = g_pRender->GetDisplayWidth();
	float fbHeight = g_pRender->GetDisplayHeight();
	m_cropWidth = fbWidth - (int)fbWidth % 8;
	m_cropHeight = fbHeight - (int)fbHeight % 8;

	unsigned int rttFlags = QTEXTURE_FILTER_TRILINEAR;
	unsigned int linFlag = QTEXTURE_FILTER_LINEAR;
	ETexturePixelFormat hdr_format;
	ETexturePixelFormat luminance_format;
	bool msaa = g_pRender->IsBackbufferMultisampled();
	
	if(caps->supportsFloatingPointTargets && caps->supportsFloatingPointLuminance)
	{
		hdr_format = QTEXTURE_FORMAT_RGBA16F;
		luminance_format = QTEXTURE_FORMAT_I32F;
	}
	else
	{
		hdr_format = luminance_format = QTEXTURE_FORMAT_RGBA8;
		m_bUsingLogLuv = true;
	}
	m_sceneHDR = g_pRender->AddRenderTarget( linFlag, fbWidth, fbHeight, hdr_format, msaa);

	// obtain scaled down version of the full scale HDR scene //
	m_scaledHDRScene = g_pRender->AddRenderTarget( linFlag, m_cropWidth / 8, m_cropHeight / 8, hdr_format );
	if( m_scaledHDRScene == QRENDER_INVALID_HANDLE )
		return FALSE;
			
	// obtain RGBA8 integer format texture for bright pass //
	m_brightPass = g_pRender->AddRenderTarget( linFlag, m_cropWidth / 8 + 2, m_cropHeight / 8 + 2, hdr_format );
	if( m_brightPass == QRENDER_INVALID_HANDLE )
		return FALSE;
		
	// obtain RGBA8 integer format texture for bloom source //
	m_bloom = g_pRender->AddRenderTarget( linFlag, m_cropWidth / 16 + 2, m_cropHeight / 16 + 2, hdr_format );
	if( m_bloom == QRENDER_INVALID_HANDLE )
		return FALSE;
		
	// obtain temporary bloom textures for bloom accumulation //
	for( int i = 1; i < 3; ++i )
	{
		m_tempBloom[i] = g_pRender->AddRenderTarget( linFlag, m_cropWidth / 16 + 2, m_cropHeight / 16 + 2, hdr_format );
		if( m_tempBloom[i] == QRENDER_INVALID_HANDLE )
			return FALSE;
	}
	
	// obtain final bloom texture //
	m_tempBloom[0] = g_pRender->AddRenderTarget( linFlag, m_cropWidth / 16, m_cropHeight / 16, hdr_format );
	if( m_tempBloom[0] == QRENDER_INVALID_HANDLE )
		return FALSE;
		
	// obtain sequentially scaled down I32F or I16F luminance textures //
	rttFlags = QTEXTURE_FILTER_NEAREST;
	for( int i = 0; i < 4; ++i )
	{
		int sampleLen = 1 << ( 2 * i );
		m_luminance[i] = g_pRender->AddRenderTarget(rttFlags, sampleLen, sampleLen, m_luminanceFormat);
		if( m_luminance[i] == QRENDER_INVALID_HANDLE )
			return FALSE;
	}
	
	// obtain adapted luminance target //
	m_lumAdaptCur = g_pRender->AddRenderTarget(rttFlags, 1, 1, m_luminanceFormat);
	if( m_lumAdaptCur == QRENDER_INVALID_HANDLE )
		return FALSE;
	
	// obtain last adapted luminance target //
	m_lumAdaptLast = g_pRender->AddRenderTarget(rttFlags, 1, 1, m_luminanceFormat);
	if( m_lumAdaptLast == QRENDER_INVALID_HANDLE )
		return FALSE;
		
	// obtain a scaled down version of the bloom texture //
	rttFlags = QTEXTURE_FILTER_LINEAR;
	m_intermediateBloom = g_pRender->AddRenderTarget(rttFlags, m_cropWidth / 8 + 2, m_cropHeight / 8 + 2, hdr_format );
	if( m_intermediateBloom == QRENDER_INVALID_HANDLE )
		return FALSE;
		
	// add effect //
	m_effect = g_pRender->AddEffect("HDR.fx", "Media/Effects/");	
	
	// init variables //
	m_deltaTime = 0.0;
	m_isInitialized = TRUE;
	
	return TRUE;
}



//////////////////////////////////////////////////
// destroyHDRPipeline
VOID CHDRPipeline::Destroy()
{
	if( !m_isInitialized )
		return;

	// cleaup the mess //	
	g_pRender->UnloadRenderTarget(m_scaledHDRScene);
	for( INT i = 0; i < 4; ++i )
	{
		g_pRender->UnloadRenderTarget(m_luminance[i]);
	}
	
	for( INT i = 0; i < 3; ++i )
	{
		g_pRender->UnloadRenderTarget(m_tempBloom[i]);
	}
	
	g_pRender->UnloadRenderTarget( m_brightPass );
	g_pRender->UnloadRenderTarget( m_bloom );
	g_pRender->UnloadRenderTarget( m_lumAdaptLast );
	g_pRender->UnloadRenderTarget( m_lumAdaptCur );
	g_pRender->UnloadRenderTarget( m_intermediateBloom );
	g_pRender->UnloadRenderTarget( m_sceneHDR );
	
	m_isInitialized = FALSE;
	m_adaptedTimer.Stop();
}



////////////////////////////////////////////////////////////////////////////
// calculateAdaptation
// Adaptive pass which uses a timer to determine light adaptation
// simulating human eye reaction to intense light sources.
// Uses 1x1 Luminance source texture and makes a pass into a 
// temporary texture downscaling the luminance value by the adaptive time
VOID CHDRPipeline::CalculateAdaptation()
{
	SQuadrionDeviceCapabilities* caps = (SQuadrionDeviceCapabilities*)g_pRender->GetDeviceCapabilities();
	
	// we need to swap the last and current luminance textures for sampling //
	INT swapTex = m_lumAdaptLast;
	m_lumAdaptLast = m_lumAdaptCur;
	m_lumAdaptCur = swapTex;
	
	// obtain an elapsed time in milliseconds //
	if( m_adaptedTimer.IsRunning )
	{
		m_deltaTime = m_adaptedTimer.GetElapsedSec();

		m_adaptedTimer.Stop();
		m_adaptedTimer.Start();
		
	}
	else
	{
		m_adaptedTimer.Start();
		m_deltaTime = 0.01F;
	}

	
	
	
	BOOL useFloatLum = !m_bUsingLogLuv;
	FLOAT fDeltaTime = (FLOAT)m_deltaTime;
	
	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("calculateAdaptationTechnique");
	hEffect->UploadParameters("g_elapsedTime", QEFFECT_VARIABLE_FLOAT, 1, &fDeltaTime);
	hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLum);
	hEffect->UploadParameters( "g_adaptationFactor", QEFFECT_VARIABLE_FLOAT, 1, &m_adaptationFactor );
	
	CQuadrionRenderTarget* rt = g_pRender->GetRenderTarget(m_lumAdaptCur);
	rt->BindRenderTarget(0);
	
	rt = g_pRender->GetRenderTarget(m_lumAdaptLast);
	rt->BindTexture(0);
	rt = g_pRender->GetRenderTarget(m_luminance[0]);
	rt->BindTexture(1);

	cTextureRect r;
	r.leftU = 0.0F;
	r.topV = 0.0F;
	r.rightU = 1.0F;
	r.bottomV = 1.0F;
	hEffect->RenderEffect(r, 1, 1);
	hEffect->EndEffect();
	
	g_pRender->EvictTextures();
}



////////////////////////////////////////////////////////////////////////
//measureLuminance
//Luminance pass, takes the scaled floating point scene texture
//and subdivides it by 4 in each direction sampling and accumulating
//avg luminance until the dest texture is 1 pixel in size, representing
//the avg luminance for the scene
VOID CHDRPipeline::MeasureLuminance()
{
	SQuadrionDeviceCapabilities* caps = (SQuadrionDeviceCapabilities*)g_pRender->GetDeviceCapabilities();
	INT i, x, y, index;
	vec2f sampleOffsets[16];
	INT curTexture = 3;  // 4 - 1 
	
	CQuadrionRenderTarget* lumCur, *scaledScene, *lum0, *lum1, *lumCurP1;
	lumCur = g_pRender->GetRenderTarget(m_luminance[curTexture]);
	scaledScene = g_pRender->GetRenderTarget(m_scaledHDRScene);
	lum0 = g_pRender->GetRenderTarget(m_luminance[0]);
	lum1 = g_pRender->GetRenderTarget(m_luminance[1]);
	
	
	// we will render to the highest luminance texture first //
	float curTexWidth = (float)lumCur->GetWidth();
	float curTexHeight = (float)lumCur->GetHeight();
	
	// calculate texel sample offsets //
	FLOAT tu, tv;
	tu = 1.0F / ( 3.0F * curTexWidth );
	tv = 1.0F / ( 3.0F * curTexHeight );	

	index = 0;

	// generate sample offsets //
	for( x = -1; x <= 1; ++x )
	{
		for( y = -1; y <= 1; ++y )
		{
			sampleOffsets[index].x = x * tu;
			sampleOffsets[index].y = y * tv;
			++index;
		}
	}
	
	// render via initial luminance pass //
	BOOL useFloatLuminance = (caps->supportsFloatingPointLuminance) ? TRUE : FALSE;
	
	CQuadrionEffect* hEffect = g_pRender->GetEffect("HDR.fx", "Media/Effects/");
	hEffect->BeginEffect("sampleLuminanceTechnique");
	hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLuminance);
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);
	hEffect->UploadParameters("g_dispWidth", QEFFECT_VARIABLE_FLOAT, 1, &curTexWidth);
	hEffect->UploadParameters("g_dispHeight", QEFFECT_VARIABLE_FLOAT, 1, &curTexHeight);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_FLOAT, 1, &m_bUsingLogLuv );

	lumCur->BindRenderTarget(0);
	scaledScene->BindTexture(0);  

   cTextureRect rect;
   rect.leftU = 0.0F;
   rect.topV = 0.0F;
   rect.rightU = 1.0F;
   rect.bottomV = 1.0F;
   hEffect->RenderEffect(rect, curTexWidth, curTexHeight);
   hEffect->EndEffect();
	
	--curTexture;
	
	// Now subdivide original scaled luminance texture down to 1 pixel //
	hEffect->BeginEffect("resampleLuminanceTechnique");
	while( curTexture > 0 )
	{
		lumCurP1 = g_pRender->GetRenderTarget(m_luminance[curTexture + 1]);
		lumCur = g_pRender->GetRenderTarget(m_luminance[curTexture]);
		curTexWidth = (float)lumCurP1->GetWidth();
		curTexHeight = (float)lumCurP1->GetHeight();
		
		QMATH_GET_SAMPLE4X4_OFFSETS(curTexWidth, curTexHeight, sampleOffsets);
		
		hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLuminance);
		hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);

		lumCur->BindRenderTarget(0);
		lumCur->Clear();
		lumCurP1->BindTexture(0);

		hEffect->RenderEffect(rect, lumCur->GetWidth(), lumCur->GetHeight());
		
		--curTexture;
	}
	hEffect->EndEffect();
	
	
	// last pass to render final luminance to a 1 pixel I32F or I16F texture //
	curTexWidth = (float)lum1->GetWidth();
	curTexHeight = (float)lum1->GetHeight();
	QMATH_GET_SAMPLE4X4_OFFSETS(curTexWidth, curTexHeight, sampleOffsets);

	hEffect->BeginEffect("finalLuminanceTechnique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);
	hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLuminance);

    lum0->BindRenderTarget(0);
    lum1->BindTexture(0);

	hEffect->RenderEffect(rect, 1.0, 1.0);
	hEffect->EndEffect();
	g_pRender->EvictTextures();
}



//////////////////////////////////////////////////////////////////////
//brightPass
//Takes average luminance along with scaled HDR scene texture to
//perform a bright pass on the entire scene
VOID CHDRPipeline::BrightPass()
{
	CQuadrionRenderTarget* bright, *scaled, *lumCur;
	bright = g_pRender->GetRenderTarget(m_brightPass);
	scaled = g_pRender->GetRenderTarget(m_scaledHDRScene);
	lumCur = g_pRender->GetRenderTarget(m_lumAdaptCur);
	
	SQuadrionDeviceCapabilities* caps = (SQuadrionDeviceCapabilities*)g_pRender->GetDeviceCapabilities();
	FLOAT brightPassWidth = bright->GetWidth();
	FLOAT brightPassHeight = bright->GetHeight();
	FLOAT scaledSceneWidth = scaled->GetWidth();
	FLOAT scaledSceneHeight = scaled->GetHeight();
   
    RECT src;
    cTextureDims srcDims;
    srcDims.height = scaledSceneWidth;
    srcDims.width = scaledSceneHeight;
    QMATH_GET_TEXTURE_RECT(srcDims, &src);
	InflateRect(&src, -1, -1);
	
	RECT dest;
	cTextureDims destDims;
	destDims.width = brightPassWidth;
	destDims.height = brightPassHeight;
	QMATH_GET_TEXTURE_RECT(destDims, &dest);
	InflateRect(&dest, -1, -1);

	cTextureRect coordRect;
	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, &dest, &coordRect);


	BOOL useFloatLuminance = (caps->supportsFloatingPointLuminance) ? useFloatLuminance = TRUE : useFloatLuminance = FALSE;;
	FLOAT middleGrey = 2.6F;   //2.6
	
	
	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("brightPassTechnique");
	hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLuminance);
	hEffect->UploadParameters("g_middleGrey", QEFFECT_VARIABLE_FLOAT, 1, &m_middleGrey);
	hEffect->UploadParameters("g_dispWidth", QEFFECT_VARIABLE_FLOAT, 1, &scaledSceneWidth);
	hEffect->UploadParameters("g_dispHeight", QEFFECT_VARIABLE_FLOAT, 1, &scaledSceneHeight);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &m_bUsingLogLuv );
	hEffect->UploadParameters("g_brightnessOffset", QEFFECT_VARIABLE_FLOAT, 1, &m_brightnessOffset);
	hEffect->UploadParameters("g_brightnessThreshold", QEFFECT_VARIABLE_FLOAT, 1, &m_brightnessThreshold);
	
	bright->BindRenderTarget(0);
	scaled->BindTexture(0);
	lumCur->BindTexture(1);

//	g_pRender->EnableScissorTest();
//	g_pRender->SetScissorRect(dest);

	hEffect->RenderEffect(coordRect, brightPassWidth, brightPassHeight);
	
//	g_pRender->DisableScissorTest();
	hEffect->EndEffect();
	g_pRender->EvictTextures();
}



///////////////////////////////////////////////////////////////
//applyBloom
//Creates the source bloom texture by which the bloom texture
//will be used to create 4 intermediate textures representing
//-x, x, -y, y samples of the original and will be stored
//in the final temporary bloom texture
VOID CHDRPipeline::ApplyBloom()
{
	CQuadrionRenderTarget* bloom, *intermediateBloom, *bright;
	bloom = g_pRender->GetRenderTarget(m_bloom);
	intermediateBloom = g_pRender->GetRenderTarget(m_intermediateBloom);
	bright = g_pRender->GetRenderTarget(m_brightPass);

	vec2f sampOffsets[16];
	vec4f sampWeights[16];
	
	FLOAT intermediateBloomWidth = intermediateBloom->GetWidth();
	FLOAT intermediateBloomHeight = intermediateBloom->GetHeight();

	RECT src, dest;
	INT nPasses;
	cTextureRect coords;
	cTextureDims destDims, srcDims;
	destDims.width = intermediateBloomWidth;
	destDims.height = intermediateBloomHeight;
	QMATH_GET_TEXTURE_RECT(destDims, &dest);
	InflateRect( &dest, -1, -1 );
	QMATH_GET_TEXTURE_COORDINATES(srcDims, NULL, destDims, &dest, &coords);

	FLOAT brightSurfWidth = bright->GetWidth();
	FLOAT brightSurfHeight = bright->GetHeight();
	QMATH_GET_GAUSSIAN5X5_OFFSETS(brightSurfWidth, brightSurfHeight, sampOffsets, sampWeights, 1.0F);

	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("gaussBlurTechnique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampOffsets);
	hEffect->UploadParameters("g_staticSampleWeights", QEFFECT_VARIABLE_FLOAT_ARRAY, 4 * 16, sampWeights);
	hEffect->UploadParameters("g_dispWidth", QEFFECT_VARIABLE_FLOAT, 1, &intermediateBloomWidth);
	hEffect->UploadParameters("g_dispHeight", QEFFECT_VARIABLE_FLOAT, 1, &intermediateBloomHeight);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &m_bUsingLogLuv );
	
	intermediateBloom->BindRenderTarget(0);
	bright->BindTexture(0);

//	g_pRender->EnableScissorTest();
//	g_pRender->SetScissorRect(dest);
	hEffect->RenderEffect(coords, intermediateBloomWidth, intermediateBloomHeight);
//	g_pRender->DisableScissorTest();
	hEffect->EndEffect();
	
	FLOAT bloomSurfWidth = bloom->GetWidth();
	FLOAT bloomSurfHeight = bloom->GetHeight();

	destDims.width = bloomSurfWidth;
	destDims.height = bloomSurfHeight;
	QMATH_GET_TEXTURE_RECT(destDims, &dest);
	InflateRect( &dest, -1, -1 );
	
	srcDims.width = intermediateBloomWidth;
	srcDims.height = intermediateBloomHeight;
	QMATH_GET_TEXTURE_RECT(srcDims, &src);
	InflateRect( &src, -1, -1 );
	
	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, &dest, &coords);
	QMATH_GET_SAMPLE2X2_OFFSETS(brightSurfWidth, brightSurfHeight, sampOffsets);

	hEffect->BeginEffect("downScale2x2Technique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampOffsets);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &m_bUsingLogLuv );

	bloom->BindRenderTarget(0);
//	g_pRender->EnableScissorTest();
//	g_pRender->SetScissorRect(dest);
	g_pRender->ClearBuffers(QRENDER_CLEAR_BACKBUFFER, 0x00000000, 0.0);
//	g_pRender->DisableScissorTest();
	intermediateBloom->BindTexture(0);
	
	
	hEffect->RenderEffect(coords, bloomSurfWidth, bloomSurfHeight);
	hEffect->EndEffect();
	g_pRender->EvictTextures();
}



////////////////////////////////////////////////////////////////
//renderBloom
//renders original bloom texture to 4 offsets and accumulates
//the result into the last temporary bloom texture
VOID CHDRPipeline::RenderBloom()
{
	vec2f sampleOffsets[16];
	vec4f sampleWeights[16];
	FLOAT fSampleOffsets[16];
	
	CQuadrionRenderTarget* tBloom1, *tBloom2, *bloom, *tBloom0;
	tBloom1 = g_pRender->GetRenderTarget(m_tempBloom[1]);
	tBloom2 = g_pRender->GetRenderTarget(m_tempBloom[2]);
	bloom = g_pRender->GetRenderTarget(m_bloom);
	tBloom0 = g_pRender->GetRenderTarget(m_tempBloom[0]);
	tBloom0->Clear();
	
	FLOAT tempBloomWidth1 = tBloom1->GetWidth();
	FLOAT tempBloomHeight1 = tBloom1->GetHeight();
	
	FLOAT tempBloomWidth2 = tBloom2->GetWidth();
	FLOAT tempBloomHeight2 = tBloom2->GetHeight();
	
	FLOAT bloomWidth = bloom->GetWidth();
	FLOAT bloomHeight = bloom->GetHeight();
	
	float bloomWidth0 = tBloom0->GetWidth();
	float bloomHeight0 = tBloom0->GetHeight();

	RECT src;
	cTextureDims srcDims, destDims;
	srcDims.width = bloomWidth;
	srcDims.height = bloomHeight;
	QMATH_GET_TEXTURE_RECT(srcDims, &src);
	InflateRect( &src, -1, -1 );

	RECT dest;
	destDims.width = tempBloomWidth2;
	destDims.height = tempBloomHeight2;
	QMATH_GET_TEXTURE_RECT(destDims, &dest);
	InflateRect( &dest, -1, -1 );

	cTextureRect coords;
	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, &dest, &coords);
	QMATH_GET_GAUSSIAN5X5_OFFSETS(bloomWidth, bloomHeight, sampleOffsets, sampleWeights, 1.0F);
   
	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("gaussBlurTechnique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);
	hEffect->UploadParameters("g_staticSampleWeights", QEFFECT_VARIABLE_FLOAT_ARRAY, 4 * 16, sampleWeights);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &m_bUsingLogLuv );
	
	tBloom2->BindRenderTarget(0);
	bloom->BindTexture(0);
	
//	g_pRender->EnableScissorTest();
//	g_pRender->SetScissorRect(dest);
	hEffect->RenderEffect(coords, tempBloomWidth2, tempBloomHeight2);
	hEffect->EndEffect();
//	g_pRender->DisableScissorTest();
	
	QMATH_GET_BLOOM_OFFSETS(tempBloomWidth2, fSampleOffsets, sampleWeights, 3.0F, 2.0F);
	
	for( INT i = 0; i < 16; ++i )
	{
		sampleOffsets[i].x = fSampleOffsets[i];
		sampleOffsets[i].y = 0.0F;
	}
	
	hEffect->BeginEffect("bloomTechnique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);
	hEffect->UploadParameters("g_staticSampleWeights", QEFFECT_VARIABLE_FLOAT_ARRAY, 4 * 16, sampleWeights);
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &m_bUsingLogLuv );

	tBloom1->BindRenderTarget(0);
	tBloom2->BindTexture(0);
   
//  g_pRender->EnableScissorTest();
//  g_pRender->SetScissorRect(dest);
	hEffect->RenderEffect(coords, tempBloomWidth1, tempBloomHeight1);
//	g_pRender->DisableScissorTest();

	g_pRender->EvictRenderTarget(0);
	g_pRender->EvictTextures();

    QMATH_GET_BLOOM_OFFSETS(tempBloomHeight1, fSampleOffsets, sampleWeights, 3.0F, 2.0F);
    for( INT i = 0; i < 16; ++i )
    {
 		sampleOffsets[i].x = 0.0F; 
		sampleOffsets[i].y = fSampleOffsets[i];
    }
	
	srcDims.width = tempBloomWidth1;
	srcDims.height = tempBloomHeight1;
	QMATH_GET_TEXTURE_RECT(srcDims, &src);
	InflateRect( &src, -1, -1 );
	destDims.width = bloomWidth0;
	destDims.height = bloomHeight0;

	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, NULL, &coords);

	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, sampleOffsets);
	hEffect->UploadParameters("g_staticSampleWeights", QEFFECT_VARIABLE_FLOAT_ARRAY, 4 * 16, sampleWeights);

	tBloom0->BindRenderTarget(0);
	tBloom1->BindTexture(0);

	hEffect->RenderEffect( coords, bloomWidth0, bloomHeight0 );
	hEffect->EndEffect();
	
	g_pRender->EvictRenderTarget(0);
	g_pRender->EvictTextures();
}



///////////////////////////////////////////////////////////////////////////////////
// renderSoftenFilter
// Proposed as an alternative to bloom which can be a bit overbearing.
// This is a simple nearest neighbor soften filter which is applied to the 
// bloom source acquired from the bright pass.
VOID CHDRPipeline::RenderSoftenFilter()
{
	CQuadrionRenderTarget* tbloom0 = g_pRender->GetRenderTarget( m_tempBloom[0] );
	CQuadrionRenderTarget* bloom   = g_pRender->GetRenderTarget( m_bloom );
	INT tempBloomWidth0 = tbloom0->GetWidth();
	INT tempBloomHeight0 = tbloom0->GetHeight();
	INT bloomSrcWidth = bloom->GetWidth();
	INT bloomSrcHeight = bloom->GetHeight();
	
	RECT src, dest;
	cTextureDims srcDims, destDims;
	srcDims.width = bloomSrcWidth;
	srcDims.height = bloomSrcHeight;
	QMATH_GET_TEXTURE_RECT(srcDims, &src);
	InflateRect( &src, -1, -1 );
	destDims.width = tempBloomWidth0;
	destDims.height = tempBloomHeight0;
	QMATH_GET_TEXTURE_RECT(destDims, &dest);
	InflateRect( &dest, -1, -1 );
	
	cTextureRect coords;
	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, &dest, &coords);
	
	FLOAT tu = 1.0F / bloomSrcWidth;
	FLOAT tv = 1.0F / bloomSrcHeight;
	
	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("softenTechnique");
	hEffect->UploadParameters("g_tu", QEFFECT_VARIABLE_FLOAT, 1, &tu);
	hEffect->UploadParameters("g_tv", QEFFECT_VARIABLE_FLOAT, 1, &tv);
	
	tbloom0->BindRenderTarget(0);
	bloom->BindTexture(0);
	g_pRender->EnableScissorTest();
	g_pRender->SetScissorRect( dest );

	hEffect->RenderEffect(coords, tempBloomWidth0, tempBloomHeight0);
	hEffect->EndEffect();

	g_pRender->EvictRenderTarget(0);
	g_pRender->EvictTextures();
}


/////////////////////////////////////////////////////////////////////
//renderHDRScene
//Final pass for hDR scene, will store the result in the frame buffer
VOID CHDRPipeline::Render( const CQuadrionRenderTarget* dest )
{	
	bool msaa = (g_pRender->GetCurrentMSAA() > 0) ? true : false;
	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DISABLE);
	if(msaa)
	{
		int db = g_pRender->GetNonMSAADepthStencil();
		CQuadrionDepthStencilTarget* ds = g_pRender->GetDepthStencilTarget(db);
		ds->BindDepthStencilTarget();
		g_pRender->DisableFSAA();
	}
	
	SQuadrionDeviceCapabilities* caps = (SQuadrionDeviceCapabilities*)g_pRender->GetDeviceCapabilities();
	RECT src;
	float dWidth = g_pRender->GetDisplayWidth();
	float dHeight = g_pRender->GetDisplayHeight();
	src.left = ( dWidth - m_cropWidth ) / 2;
	src.top = ( dHeight - m_cropHeight ) / 2;
	src.right = src.left + m_cropWidth;
	src.bottom = src.top + m_cropHeight;
	
	CQuadrionRenderTarget* scene, *scaled, *tBloom0, *lumCur;
	scene = g_pRender->GetRenderTarget(m_sceneHDR);
	scaled = g_pRender->GetRenderTarget(m_scaledHDRScene);
	tBloom0 = g_pRender->GetRenderTarget(m_tempBloom[0]);
	lumCur = g_pRender->GetRenderTarget(m_luminance[0]);
	
	cTextureRect texRec;
	cTextureDims srcDims, destDims;
	srcDims.width = scene->GetWidth();
	srcDims.height = scene->GetHeight();
	QMATH_GET_TEXTURE_COORDINATES(srcDims, &src, destDims, NULL, &texRec);
	
	vec2f avSamples[16];
	INT hdrWidth = scene->GetWidth();
	INT hdrHeight = scene->GetHeight();
	QMATH_GET_SAMPLE4X4_OFFSETS(hdrWidth, hdrHeight, avSamples);

	BOOL encodeLL = m_bUsingLogLuv;//( caps->maxFSAA == 0 );
	CQuadrionEffect* hEffect = g_pRender->GetEffect(m_effect);
	hEffect->BeginEffect("downScale4x4Technique");
	hEffect->UploadParameters("g_staticSampleOffsets", QEFFECT_VARIABLE_FLOAT_ARRAY, 2 * 16, avSamples);
	hEffect->UploadParameters("g_dispWidth", QEFFECT_VARIABLE_FLOAT, 1, &dWidth);
	hEffect->UploadParameters("g_dispHeight", QEFFECT_VARIABLE_FLOAT, 1, &dHeight);
	hEffect->UploadParameters("g_bEncodeLogLuv", QEFFECT_VARIABLE_FLOAT, 1, &encodeLL);
	
	scaled->BindRenderTarget(0);
	scene->BindTexture(0);
	
	if(msaa)
	{
		CQuadrionDepthStencilTarget* ds = NULL;
		int db = g_pRender->GetNonMSAADepthStencil();
		ds = g_pRender->GetDepthStencilTarget(db);
		ds->BindDepthStencilTarget();
	}
	
	else
	{
		g_pRender->SetDefaultDepthStencilTarget();
	}
	
	unsigned int scaledWidth, scaledHeight;
	scaledWidth = scaled->GetWidth();
	scaledHeight = scaled->GetHeight();
	hEffect->RenderEffect(texRec, scaledWidth, scaledHeight);
	hEffect->EndEffect();
	
	// Create luminance textures //
	MeasureLuminance();
	
	
	// Create adaptive luminance //
	CalculateAdaptation();
	
	// Perform bright pass //
	BrightPass();
	
	
	// Create bloom source  from bright pass //
	ApplyBloom();
	
	
	// Render bloom to texture or soften filter //
	RenderBloom();
//	RenderSoftenFilter();
	
	
	BOOL useFloatLuminance = !m_bUsingLogLuv;
	FLOAT middleGrey = 2.6F;			//2.6
	FLOAT bloomScale = 1.0F;
	FLOAT dw, dh;
	
	if( !dest )
	{
		dw = g_pRender->GetDisplayWidth();
		dh = g_pRender->GetDisplayHeight();
		g_pRender->SetDefaultRenderTarget();	
		g_pRender->SetDefaultDepthStencilTarget();
	}
	
	else
	{
		dw = ((CQuadrionTextureObject*)dest)->GetWidth();
		dh = ((CQuadrionTextureObject*)dest)->GetHeight();
		((CQuadrionRenderTarget*)dest)->BindRenderTarget(0);
		g_pRender->SetDefaultDepthStencilTarget();
	}
	
	
	hEffect->BeginEffect("finalTechnique");
	hEffect->UploadParameters("g_useFloatLuminance", QEFFECT_VARIABLE_BOOL, 1, &useFloatLuminance);
	hEffect->UploadParameters("g_middleGrey", QEFFECT_VARIABLE_FLOAT, 1, &m_finalGrey);
	hEffect->UploadParameters("g_bloomScale", QEFFECT_VARIABLE_FLOAT, 1, &m_bloomScale);
	hEffect->UploadParameters("g_dispWidth", QEFFECT_VARIABLE_FLOAT, 1, &dw);
	hEffect->UploadParameters("g_dispHeight", QEFFECT_VARIABLE_FLOAT, 1, &dh);	
	hEffect->UploadParameters( "g_bEncodeLogLuv", QEFFECT_VARIABLE_BOOL, 1, &encodeLL );	

	g_pRender->ClearBuffers(QRENDER_CLEAR_BACKBUFFER | QRENDER_CLEAR_ZBUFFER, 0xFFFFFFFF, 1.0);
	scene->BindTexture(0);
	tBloom0->BindTexture(1);
	lumCur->BindTexture(2);
	
	cTextureRect rect;
	rect.leftU = 0.0F;
	rect.topV = 0.0F;
	rect.rightU = 1.0F;
	rect.bottomV = 1.0F;
	
	if(msaa)
		g_pRender->EnableFSAA();
	hEffect->RenderEffect(rect, dw, dh);
	hEffect->EndEffect();

	g_pRender->ChangeDepthMode(QRENDER_ZBUFFER_DEFAULT);
	g_pRender->EvictTextures();
	if( dest )
		g_pRender->EvictRenderTarget(0);
}

void CHDRPipeline::ClearTargets()
{
	CQuadrionRenderTarget* rt;
//	rt = g_pRender->GetRenderTarget(m_sceneHDR);
//	rt->Clear();
	rt = g_pRender->GetRenderTarget(m_scaledHDRScene);
	rt->Clear();
	for(int i = 0; i < 4; ++i)
	{
		rt = g_pRender->GetRenderTarget(m_luminance[i]);
		rt->Clear();
	}
	
	rt = g_pRender->GetRenderTarget(m_brightPass);
	rt->Clear();
	rt = g_pRender->GetRenderTarget(m_bloom);
	rt->Clear();
	
	for(int i = 0; i < 3; ++i)
	{
		rt = g_pRender->GetRenderTarget(m_tempBloom[i]);
		rt->Clear();
	}
	
	rt = g_pRender->GetRenderTarget(m_intermediateBloom);
	rt->Clear();
}