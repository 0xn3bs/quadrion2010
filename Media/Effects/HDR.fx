#include "Encoding.fx"



static const int		MAX_SAMPLES   = 16;

static const float3  LUM_VEC		  = float3( 0.299F, 0.587F, 0.114F );
static const float3  LUM_CONV		  = float3(0.2126, 0.7152, 0.0722);
static const float   BRIGHT_THRESHOLD = 2.0F;    //5
static const float	 MAX_LUMINANCE    = 20.0F;	// 100
static const float3  BLUE_SHIFT		  = float3( 1.05F, 0.97F, 1.27F );



bool		g_blueShift;
float		g_bloomScale;
float		g_middleGrey;
float		g_elapsedTime;
float2		g_staticSampleOffsets[MAX_SAMPLES];
float4		g_staticSampleWeights[MAX_SAMPLES];
float		g_tu;
float		g_tv;
float		g_dispWidth;
float		g_dispHeight;
bool		g_useFloatLuminance;
bool		g_bEncodeLogLuv;

float		g_brightnessOffset;
float		g_brightnessThreshold;
float		g_adaptationFactor;





// Texture sampler setup //
sampler s0 : register( s0 );
sampler s1 : register( s1 );
sampler s2 : register( s2 );






//////////////////////////////////////////////////////
// downScale4x4
// Downscales original full resolution HDR texture by
// 16 for luminance calc.
float4 downScale4x4( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp = 0.0F;
	
	for( int i = 0; i < 16; i++ )
	{
		if(!g_bEncodeLogLuv)
			samp += tex2D( s0, screenPos + g_staticSampleOffsets[i] );
		else
			samp += float4(decodeLogLuv(tex2D(s0, screenPos + g_staticSampleOffsets[i])), 1.0);
	}

//	if(!g_bEncodeLogLuv)
	//	return samp / 16;	
//	else
	if( g_bEncodeLogLuv )
		return encodeLogLuv(samp.rgb / 16.0);
	else
		return samp / 16.0;
//	return encodeLogLuv(samp.rgb / 16);
}



float4 downScale2x2( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp = 0.0F;
	for( int i = 0; i < 4; ++i )
	{
		if( g_bEncodeLogLuv )
			samp += float4(decodeLogLuv(tex2D( s0, screenPos + g_staticSampleOffsets[i] )), 1.0);
		else
			samp += tex2D( s0, screenPos + g_staticSampleOffsets[i] );
	}
	
//	return samp / 4;
	if( g_bEncodeLogLuv )
		return encodeLogLuv(samp.rgb / 4.0);
	else
		return samp / 4.0;
}


///////////////////////////////////////////////////////////////////
// sampleLuminance
// samples downscaled FP render target into a I32 luminance texture
// of the same dimension
float4 sampleLuminance( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp;
	float logLumSamp = 0.0F;
	
	for( int i = 0; i < 9; ++i )
	{
		samp = tex2D( s0, screenPos + g_staticSampleOffsets[i] );
		if( g_bEncodeLogLuv )
			samp.rgb = decodeLogLuv(samp);
		logLumSamp += log( dot( samp.rgb, LUM_CONV ) + 0.0001F );
	}
	
	logLumSamp /= 9;
	if(logLumSamp > MAX_LUMINANCE)
		logLumSamp = -1.0;
	
	if(g_useFloatLuminance)
		return float4( logLumSamp, 1.0, 1.0, 1.0F );
	else
		return encodeRE8(logLumSamp);
}


///////////////////////////////////////////////////////////////
// resampleLuminance
float4 resampleLuminance( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float resampleSum = 0.0F;
	
	for( int i = 0; i < 16; ++i )
	{
		if(g_useFloatLuminance)
			resampleSum += tex2D( s0, screenPos + g_staticSampleOffsets[i] ).r;
		else
			resampleSum += decodeRE8(tex2D(s0, screenPos + g_staticSampleOffsets[i]));
	}
	
	resampleSum /= 16;
	if(resampleSum > MAX_LUMINANCE)
		resampleSum = -1.0;
	
	if(g_useFloatLuminance)
		return float4( resampleSum, 1.0F, 1.0F, 1.0F );
	else
		return encodeRE8(resampleSum);
}


//////////////////////////////////////////////////////////////////
// finalLuminance
float4 finalLuminance( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float samp = 0.0F;
	
	for( int i = 0; i < 16; ++i )
	{
		if(g_useFloatLuminance)
			samp += tex2D( s0, screenPos + g_staticSampleOffsets[i] ).r;
		else
			samp += decodeRE8(tex2D(s0, screenPos + g_staticSampleOffsets[i]));
	}
	
	samp = exp( samp / 16 );
	if(samp > MAX_LUMINANCE)
		samp = 1.0;
	
	if(g_useFloatLuminance)
		return float4( samp, 1.0, 1.0, 1.0F );
	else
		return encodeRE8(samp);
}


float3 toneMapping(float3 vColor, float fLum)
{
	float fLumPixel = dot(vColor, LUM_CONV);
	float fLumScaled = (fLumPixel * g_middleGrey) / fLum;
	float fLumCompressed = (fLumScaled * (1 + (fLumScaled / (MAX_LUMINANCE * MAX_LUMINANCE)))) / (1 + fLumScaled);
	
	return vColor * fLumCompressed;
}



/////////////////////////////////////////////////////////////////////
// brightPass
float4 brightPass( in float2 screenPos : TEXCOORD0 ) : COLOR 
{
	float4 samp = tex2D( s0, screenPos );
	if( g_bEncodeLogLuv )
		samp.rgb = decodeLogLuv(samp);
	
	float lum;
	if(g_useFloatLuminance)
		lum = tex2D( s1, float2( 0.5F, 0.5F ) ).r;
	else
		lum = decodeRE8(tex2D(s1, float2(0.5, 0.5)));
		
	
	samp.rgb *= g_middleGrey / ( lum + 0.001F );
	samp.rgb -= g_brightnessThreshold;
	samp = max( samp, 0.0F );
	samp.rgb /= ( g_brightnessOffset + samp );
	
//	return samp;
	if( g_bEncodeLogLuv )
		return encodeLogLuv(samp.rgb);
	else
		return samp;
}



/////////////////////////////////////////////////////////////////
// gaussBlur
float4 gaussBlur( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp = 0.0F;
	for( int i = 0; i < 12; ++i )
	{
		if( g_bEncodeLogLuv )
			samp += g_staticSampleWeights[i] * float4(decodeLogLuv(tex2D( s0, screenPos + g_staticSampleOffsets[i] )), 1.0);
		else
			samp += g_staticSampleWeights[i] * tex2D( s0, screenPos + g_staticSampleOffsets[i] );
	}
	
//	return samp;
	if( g_bEncodeLogLuv )
		return encodeLogLuv(samp.rgb);
	else
		return samp;
}






/////////////////////////////////////////////////////////////////////////
// final
float4 final( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp = tex2D( s0, screenPos );
	if( g_bEncodeLogLuv )
		samp.rgb = decodeLogLuv(samp);

	float4 bloom = tex2D( s1, screenPos );
	if( g_bEncodeLogLuv )
		bloom.rgb = decodeLogLuv(bloom);
	
	float lum;
	if(g_useFloatLuminance)
		lum = tex2D( s2, float2( 0.5F, 0.5F ) ).r;
	else
		lum = decodeRE8(tex2D(s2, float2(0.5, 0.5)));
		
//	samp.rgb *= g_middleGrey / ( lum + 0.001F );
//	samp.rgb /= ( 1.0F + samp );
	samp += g_bloomScale * bloom;
	
	return samp;
}


/////////////////////////////////////////////////////////////////
// calculateAdaptation
// Calculates an average luminance to a 1x1 pixel texture
// from the 1x1 luminance sample and a delta time in milliseconds
float4 calculateAdaptation( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float adaptedLum, curLum;
	if(g_useFloatLuminance)
	{
		adaptedLum = tex2D( s0, float2( 0.5F, 0.5F ) ).r;
		curLum     = tex2D( s1, float2( 0.5F, 0.5F ) ).r;
	}
	else
	{
		adaptedLum = decodeRE8(tex2D(s0, float2(0.5, 0.5)));
		curLum = decodeRE8(tex2D(s1, float2(0.5, 0.5)));
	}
	
//	const float tau = 0.1F;
//	float fAdaptedLum = adaptedLum + (curLum - adaptedLum) * (1.0 - exp(-g_elapsedTime * tau));
	
//	if(g_useFloatLuminance)
//		return float4(fAdaptedLum, 1.0, 1.0, 1.0);
//	else
//		return encodeRE8(fAdaptedLum);

	// Check for bad values and peg them //
	if( adaptedLum > MAX_LUMINANCE )
		adaptedLum = -2.0;
	if( curLum > MAX_LUMINANCE )
		curLum = -2.0;
	
	float newAdaptation = adaptedLum + ( curLum - adaptedLum ) * ( 1.0 - pow( 0.98F, g_adaptationFactor * g_elapsedTime ) );
	
	if(g_useFloatLuminance)
		return float4( newAdaptation, 1.0F, 1.0F, 1.0F );
	else
		return encodeRE8(newAdaptation);
}


///////////////////////////////////////////////////////
// bloom
// Renders the bloom source from bright pass
float4 bloom( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 asamp = 0.0F;
	float2 sampPos;
	
	for( int i = 0; i < 12; ++i )
	{
		sampPos = screenPos + g_staticSampleOffsets[i];
		if( g_bEncodeLogLuv )
			asamp += g_staticSampleWeights[i] * float4( decodeLogLuv( tex2D( s0, sampPos ) ), 1.0 );
		else
			asamp += g_staticSampleWeights[i] * tex2D( s0, sampPos );
	}
	
	if( g_bEncodeLogLuv )
		return encodeLogLuv(asamp.rgb);
	else
		return asamp;
}


///////////////////////////////////////////////////////////////
// soften
// In opposition to bloom, will simply just soften the image
float4 soften( in float2 screenPos : TEXCOORD0 ) : COLOR
{
	float4 samp = 0.0F;
	float4 color = 0.0F;
	float2 sampPos = screenPos;
	
	for( int i = 0; i < 4; ++i )
	{
		if( i == 0 )
			sampPos.x += g_tu;
		if( i == 1 )
			sampPos.x -= g_tu * 2;
		if( i == 2 )
		{
			sampPos.x += g_tu;
			sampPos.y += g_tv;
		}
		
		if( i == 3 )
		{
			sampPos.y -= g_tv * 2;
		}
		
		color = tex2D( s0, sampPos );
		samp += color;
	}
	
	return samp / 4;
}



////////////////////////////////// TECHNIQUES ///////////////////////////////////


technique downScale4x4Technique
{
	pass Pass_0
	{
		AddressU[0] = Clamp;
		AddressV[0] = Clamp;
		PixelShader = compile ps_2_a downScale4x4();
	}
}

technique downScale2x2Technique
{
	pass Pass_0 
	{
		AddressU[0] = Clamp;
		AddressV[0] = Clamp;
		MinFilter[0] = Point;
		PixelShader = compile ps_2_a downScale2x2();
	}
}


technique sampleLuminanceTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Linear;
		MagFilter[0] = Linear;
		MinFilter[1] = Linear;
		MagFilter[1] = Linear;
		PixelShader = compile ps_2_a sampleLuminance();
	}
}



technique resampleLuminanceTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MinFilter[1] = Point;
		MagFilter[1] = Point;
		PixelShader = compile ps_2_a resampleLuminance();
	}
}


technique finalLuminanceTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MinFilter[1] = Point;
		MagFilter[1] = Point;
		PixelShader = compile ps_2_a finalLuminance();
	}
}


technique brightPassTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MinFilter[1] = Point;
		MagFilter[1] = Point;
		PixelShader = compile ps_2_a brightPass();
	}
}


technique gaussBlurTechnique
{
	pass Pass_0
	{
		AddressU[0] = Clamp;
		AddressV[0] = Clamp;
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		PixelShader = compile ps_2_a gaussBlur();
	}
}



technique bloomTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		AddressU[0] = Clamp;
		AddressV[0] = Clamp;
		PixelShader = compile ps_2_a bloom();
	}
}

technique softenTechnique
{
	pass Pass_0
	{
		PixelShader = compile ps_2_0 soften();
	}
}


technique finalTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MinFilter[1] = Linear;
		MagFilter[1] = Linear;
		MinFilter[2] = Point;
		MagFilter[2] = Point;
		PixelShader = compile ps_2_a final();
	}
}


technique calculateAdaptationTechnique
{
	pass Pass_0
	{
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MinFilter[1] = Point;
		MagFilter[1] = Point;
		PixelShader = compile ps_2_a calculateAdaptation();
	}
}