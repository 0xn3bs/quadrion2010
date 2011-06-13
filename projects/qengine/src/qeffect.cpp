#include "qeffect.h"
#include "qrender.h"
#include "qresource.h"







static unsigned int effectVarSizes[] = 
{
	4,
	4,
	4,
	4 * 4,
	4 * 9,
	4 * 16,
	4 * 16,
	4, 
	4, 
	4,
};

static std::string GetMatrixSemanticString(const unsigned int& matrix)
{
	std::string mString;
	
	if(matrix & QEFFECT_MATRIX_WORLD) mString = "WORLD";
	else if(matrix & QEFFECT_MATRIX_VIEW) mString = "VIEW";
	else if(matrix & QEFFECT_MATRIX_PROJECTION) mString = "PROJECTION";
	else if(matrix & QEFFECT_MATRIX_WORLDVIEW) mString = "WORLDVIEW";
	else if(matrix & QEFFECT_MATRIX_VIEWPROJECTION) mString = "VIEWPROJECTION";
	else if(matrix & QEFFECT_MATRIX_WORLDVIEWPROJECTION) mString = "WORLDVIEWPROJECTION";
	else mString = "NULL";
	
	if(matrix & QEFFECT_MATRIX_INVERSE) mString += "INVERSE";
	else if(matrix & QEFFECT_MATRIX_TRANSPOSE) mString += "TRANSPOSE";
	else if(matrix & QEFFECT_MATRIX_INVERSETRANSPOSE) mString += "INVERSETRANSPOSE";
	else mString += "";
	
	return mString;
}


static void GetStateMatrix(const LPDIRECT3DDEVICE9 dev, const void* inVal, void* outVal)
{
	D3DXMATRIX M, V, P, MOD;
	unsigned int type;
	memcpy(&type, inVal, sizeof(unsigned int));
	
		
	if(type & QEFFECT_MATRIX_WORLDVIEWPROJECTION)
	{
		dev->GetTransform(D3DTS_WORLD, &MOD);
		dev->GetTransform(D3DTS_VIEW, &V);
		dev->GetTransform(D3DTS_PROJECTION, &P);
		D3DXMatrixMultiply(&M, &V, &P);
		D3DXMatrixMultiply(&M, &MOD, &M);
	}

	else if(type & QEFFECT_MATRIX_VIEWPROJECTION)
	{
		dev->GetTransform(D3DTS_VIEW, &V);
		dev->GetTransform(D3DTS_PROJECTION, &P);
		D3DXMatrixMultiply(&M, &V, &P);	
	}

	else if(type & QEFFECT_MATRIX_WORLDVIEW)
	{
		dev->GetTransform(D3DTS_WORLD, &M);
		dev->GetTransform(D3DTS_VIEW, &V);
		D3DXMatrixMultiply(&M, &M, &V);
	}
	
	else if(type & QEFFECT_MATRIX_PROJECTION)
	{
		dev->GetTransform(D3DTS_PROJECTION, &M);
	}
	
	else if(type & QEFFECT_MATRIX_VIEW)
	{
		dev->GetTransform(D3DTS_VIEW, &M);
	}
	
	else if(type & QEFFECT_MATRIX_WORLD)
	{
		dev->GetTransform(D3DTS_WORLD, &M);
	}
	
	else 
		return;
		

	if(type & QEFFECT_MATRIX_INVERSE)
		D3DXMatrixInverse(&M, NULL, &M);
	
	if(!(type & QEFFECT_MATRIX_TRANSPOSE))
		D3DXMatrixTranspose(&M, &M);
	
	memcpy(outVal, &M.m, 4 * 16);
}


struct SScreenVertex
{
	D3DXVECTOR4		pos;
	D3DXVECTOR2		texCoords;
};

struct SScreenVertexFrustumCorners
{
	D3DXVECTOR4		pos;
	D3DXVECTOR3		norm;
	D3DXVECTOR2		texCoords;
};


#define D3DFVF_SCREENSPACEVERTEX					(D3DFVF_XYZRHW | D3DFVF_TEX1)
#define D3DFVF_SCREENSPACEVERTEX_FRUSTUMCORNERS		(D3DFVF_XYZRHW | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE3(0) | D3DFVF_TEXCOORDSIZE2(1))


static void RenderFullscreenQuad(const LPDIRECT3DDEVICE9 dev, const cTextureRect& rect, const unsigned int& width, const unsigned int& height)
{
	float fWidth5 = (float)width - 0.5F;
	float fHeight5 = (float)height - 0.5F;
	
	SScreenVertex quad[4];	
	quad[0].pos = D3DXVECTOR4(-0.5F, -0.5F, 0.5F, 1.0F);
	quad[0].texCoords = D3DXVECTOR2(rect.leftU, rect.topV);
	quad[1].pos = D3DXVECTOR4(fWidth5, -0.5F, 0.5F, 1.0F);
	quad[1].texCoords = D3DXVECTOR2(rect.rightU, rect.topV);
	quad[2].pos = D3DXVECTOR4(-0.5F, fHeight5, 0.5F, 1.0F);
	quad[2].texCoords = D3DXVECTOR2(rect.leftU, rect.bottomV);
	quad[3].pos = D3DXVECTOR4(fWidth5, fHeight5, 0.5F, 1.0F);
	quad[3].texCoords = D3DXVECTOR2(rect.rightU, rect.bottomV);
	
	dev->SetFVF(D3DFVF_SCREENSPACEVERTEX);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(SScreenVertex));
}

static void RenderFullscreenQuadFrustumCorners(const LPDIRECT3DDEVICE9 dev, const cTextureRect& rect, const unsigned int& width, const unsigned int& height, const vec3f* V)
{
	float fWidth5 = (float)width - 0.5F;
	float fHeight5 = (float)height - 0.5F;
	
	SScreenVertexFrustumCorners quad[4];
	quad[0].pos = D3DXVECTOR4( -0.5F, -0.5F, 0.5F, 1.0F );
	quad[0].texCoords = D3DXVECTOR2( rect.leftU, rect.topV );
	quad[0].norm = D3DXVECTOR3(V[0].x, V[0].y, V[0].z);
	quad[1].pos = D3DXVECTOR4( fWidth5, -0.5F, 0.5F, 1.0F );
	quad[1].texCoords = D3DXVECTOR2( rect.rightU, rect.topV );
	quad[1].norm = D3DXVECTOR3(V[1].x, V[1].y, V[1].z);
	quad[2].pos = D3DXVECTOR4( -0.5F, fHeight5, 0.5F, 1.0F );
	quad[2].texCoords = D3DXVECTOR2( rect.leftU, rect.bottomV );
	quad[2].norm = D3DXVECTOR3(V[2].x, V[2].y, V[2].z);
	quad[3].pos = D3DXVECTOR4( fWidth5, fHeight5, 0.5F, 1.0F );
	quad[3].texCoords = D3DXVECTOR2( rect.rightU, rect.bottomV );
	quad[3].norm = D3DXVECTOR3(V[3].x, V[3].y, V[3].z);
	
//	dev->SetRenderState(D3DRS_ZENABLE, false);
	dev->SetFVF(D3DFVF_SCREENSPACEVERTEX_FRUSTUMCORNERS);
	dev->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(SScreenVertexFrustumCorners));
//	dev->SetRenderState(D3DRS_ZENABLE, true);
}

//==========================================================================================================================




CQuadrionEffect::CQuadrionEffect(const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_bIsInitialized  = false;
	m_bIsShaderLoaded = false;
	m_nPasses         = 0;
	m_hStateBlock	  = NULL;
	
	for(unsigned int i = 0; i < QEFFECT_MAX_PARAMETERS; ++i)
	{
		m_effectParameterValues[i] = NULL;
	}
	
	memset(m_effectParameterTypes, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	memset(m_effectParameterCounts, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	m_nParameters = 0;
	m_bReloadStateBlock = false;
}

CQuadrionEffect::CQuadrionEffect(const void* pRender, const unsigned int handle, const std::string& name, const std::string& path) : CQuadrionResource(handle, name, path)
{
	m_bIsInitialized  = false;
	m_bIsShaderLoaded = false;
	m_nPasses         = 0;
	m_hStateBlock	  = NULL;
	ChangeRenderDevice(pRender);
	
	for(unsigned int i = 0; i < QEFFECT_MAX_PARAMETERS; ++i)
	{
		m_effectParameterValues[i] = NULL;
	}
	
	memset(m_effectParameterTypes, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	memset(m_effectParameterCounts, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	m_nParameters = 0;
	m_bReloadStateBlock = false;
}


CQuadrionEffect::~CQuadrionEffect()
{
	DestroyEffect();
}


void CQuadrionEffect::ChangeRenderDevice(const void* pRender)
{
	m_pQuadrionRender = (CQuadrionRender*)pRender;
	m_pRenderDevice = m_pQuadrionRender->m_pD3DDev;

	m_bIsInitialized = true;
}



bool CQuadrionEffect::CreateEffect(const char* fname)
{
	if(!fname)
		return false;

	LPD3DXBUFFER pCompilationErrors;
	DWORD		 buildOptions;
	std::string  effectErrorStr;
	
#ifdef _DEBUG
	buildOptions = (D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION);
#else
//	buildOptions = D3DXSHADER_OPTIMIZATION_LEVEL0;
	buildOptions = D3DXSHADER_SKIPOPTIMIZATION;
#endif

	if(FAILED(D3DXCreateEffectFromFile(m_pRenderDevice, _QT(fname), NULL, NULL, buildOptions, NULL, &m_pEffectHandle, &pCompilationErrors)))
	{	
		if(!pCompilationErrors)
			return false;

		INT bufSize = pCompilationErrors->GetBufferSize();
		LPSTR p = (LPSTR)malloc(bufSize); 
		memcpy(p, pCompilationErrors->GetBufferPointer(), bufSize);
		effectErrorStr.append("Error in FX: ");
		effectErrorStr.append(p);
		effectErrorStr.append("\n");
		free(p);
		p = NULL;
		
		MessageBoxA(NULL, effectErrorStr.c_str(), "Effect Compilation Error", MB_OK | MB_ICONERROR);
	
		return false;
	}

	m_bIsShaderLoaded = true;
	return true;
}


bool CQuadrionEffect::CreateEffect(const std::string& name, const std::string& path)
{
	std::string cName = path + name;
	return CreateEffect(cName.c_str());
}

bool CQuadrionEffect::BeginEffect(const std::string& technique)
{

	// Attempt to get a technique handle //
	// If succeeded: then bind it as current //
	m_hCurrentTechnique = m_pEffectHandle->GetTechniqueByName(technique.c_str());
	if(!m_hCurrentTechnique)
		return false;
	
	m_pEffectHandle->SetTechnique(m_hCurrentTechnique);
	m_pQuadrionRender->m_currentEffect = GetHandle();	

	return true;
}

void CQuadrionEffect::EndEffect()
{
//	if(m_hCurrentTechnique != 0)
//		m_pEffectHandle->End();

	if(m_hStateBlock != 0)
		m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
		
	m_bReloadStateBlock = true;
	m_hCurrentTechnique = 0;
	m_hStateBlock = NULL;
	m_pQuadrionRender->m_currentEffect = QRENDER_INVALID_HANDLE;
	
	m_nPasses = 0;
}

bool CQuadrionEffect::UploadParameters(const std::string& alias, const EQuadrionEffectVariableType& paramType, const unsigned int& nParameters, const void* val)
{	

	bool varFound = false;
	if(m_nParameters > 0)
	{
		// Check that parameter name already exists
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			// Parameter name does exist
			if(alias.compare(m_effectParameterNames[i]) == 0)
			{
				if(paramType != QEFFECT_VARIABLE_STATE_MATRIX)
				{
					// Check that value has changed 
					if(memcmp(m_effectParameterValues[i], val, effectVarSizes[paramType] * nParameters) != 0)
					{
						// If the size has changed, realloc the memory
						if((effectVarSizes[paramType] * nParameters) != (effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]))
						{
							if(m_effectParameterValues[i])
								m_effectParameterValues[i] = realloc(m_effectParameterValues[i], effectVarSizes[paramType] * nParameters);
							else
								m_effectParameterValues[i] = malloc(effectVarSizes[paramType] * nParameters);
						}
					
						// Copy the new value //
						memcpy(m_effectParameterValues[i], val, effectVarSizes[paramType] * nParameters);
						m_bReloadStateBlock = true;
					}
				}
				
				else
				{
					GetStateMatrix(m_pRenderDevice, val, m_effectParameterValues[i]); 
					m_bReloadStateBlock = true;
				}
				
				varFound = true;
				break;
			}
		}
		
		if(!varFound)
		{
			m_effectParameterValues[m_nParameters] = malloc(effectVarSizes[paramType] * nParameters);
			if(!m_effectParameterValues[m_nParameters])
				return false;
			
			m_effectParameterCounts[m_nParameters] = nParameters;
			m_effectParameterTypes[m_nParameters] = paramType;
			m_effectParameterNames[m_nParameters] = alias;
			
			if(paramType != QEFFECT_VARIABLE_STATE_MATRIX)
				memcpy(m_effectParameterValues[m_nParameters], val, effectVarSizes[paramType] * nParameters);
			else
				GetStateMatrix(m_pRenderDevice, val, m_effectParameterValues[m_nParameters]);
						
			++m_nParameters;
			m_bReloadStateBlock = true;
		}
	}			
	
	else
	{
		m_effectParameterValues[m_nParameters] = malloc(effectVarSizes[paramType] * nParameters);
		if(!m_effectParameterValues[m_nParameters])
			return false;
				
		m_effectParameterCounts[m_nParameters] = nParameters;
		m_effectParameterTypes[m_nParameters] = paramType;
		m_effectParameterNames[m_nParameters] = alias;
		
		if(paramType != QEFFECT_VARIABLE_STATE_MATRIX)
			memcpy(m_effectParameterValues[m_nParameters], val, effectVarSizes[paramType] * nParameters);
		else
			GetStateMatrix(m_pRenderDevice, val, m_effectParameterValues[m_nParameters]);
			
		++m_nParameters;
		m_bReloadStateBlock = true;
	}
	
	return true;
}


bool CQuadrionEffect::UploadVectorConstant(const std::string& alias, const unsigned int& nParameters, const float* val)
{

	if(nParameters <= 0 || !val)
		return false;

	D3DXHANDLE hParam = m_pEffectHandle->GetParameterBySemantic(NULL, alias.c_str());	
	if(hParam)
	{
		if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
			m_pEffectHandle->SetVectorArray(hParam, (D3DXVECTOR4*)val, nParameters);
		
		return true;
	}
	
	else
		return false;
}


bool CQuadrionEffect::UploadStateMatrix(const unsigned int& matrix)
{
	std::string mString = GetMatrixSemanticString(matrix);
	D3DXHANDLE hParam = m_pEffectHandle->GetParameterBySemantic(NULL, mString.c_str());
	mat4 M;
	
	if(hParam)
	{
		if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
		{
			GetStateMatrix(m_pRenderDevice, &matrix, &M);
			m_pEffectHandle->SetMatrix(hParam, (D3DXMATRIX*)&M);
		}
	}

	return true;	
}


bool CQuadrionEffect::UploadTexture(const std::string& paramName, const CQuadrionTextureObject* textureObject)
{

	D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, paramName.c_str());
	if(!hParam)
		return false;
	
	if(FAILED(m_pEffectHandle->SetTexture(hParam, textureObject->m_pTextureObject)))
		return false;

	return true;
}


void CQuadrionEffect::DestroyEffect()
{

	if(m_pEffectHandle)
	{
		m_pEffectHandle->Release();
		m_pEffectHandle = NULL;
	}
	m_bIsInitialized  = false;
	m_bIsShaderLoaded = false;
	m_nPasses		  = 0;
	
	for(unsigned int i = 0; i < QEFFECT_MAX_PARAMETERS; ++i)
	{
		if(m_effectParameterValues[i])
		{
			free(m_effectParameterValues[i]);
			m_effectParameterValues[i] = NULL;
		}
	}
	
	memset(m_effectParameterNames, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	memset(m_effectParameterCounts, 0, sizeof(unsigned int) * QEFFECT_MAX_PARAMETERS);
	
	m_nParameters = 0;
	m_bReloadStateBlock = false;
	m_hStateBlock = NULL;
}

void CQuadrionEffect::GetEffectName(std::string& oName)
{	

	if(m_bIsInitialized)
		oName = m_effectPath;
}


bool CQuadrionEffect::RenderEffect(void* callbackObj, void (*renderCallback)(void* callbackObj))
{
//	m_pEffectHandle->SetTechnique(m_hCurrentTechnique);

	if(m_bReloadStateBlock)
	{
		if(m_hStateBlock != NULL)
			m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
			
		m_pEffectHandle->BeginParameterBlock();
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, m_effectParameterNames[i].c_str());
			if(hParam)
			{
				if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
				{
					if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_BOOL)
						m_pEffectHandle->SetBool(hParam, *(BOOL*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_INT)
						m_pEffectHandle->SetInt(hParam, *(INT*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_FLOAT)
						m_pEffectHandle->SetFloat(hParam, *(FLOAT*)(m_effectParameterValues[i]));
					else
						m_pEffectHandle->SetValue(hParam, m_effectParameterValues[i], effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]);
				}
			}
		}		
		
		m_hStateBlock = m_pEffectHandle->EndParameterBlock();
		m_bReloadStateBlock = false;
	}

	m_pEffectHandle->ApplyParameterBlock(m_hStateBlock);

	unsigned int nPasses;
	if(FAILED(m_pEffectHandle->Begin(&nPasses, 0)))
		return false;
	
	if(nPasses > 0)
		m_nPasses = nPasses;
	
	else
		return false;

	for(unsigned int i = 0; i < m_nPasses; ++i)
	{
		m_pEffectHandle->BeginPass(i);	
		
		renderCallback(callbackObj);
		
		m_pEffectHandle->EndPass();
	}	
	
	m_pEffectHandle->End();
//	m_hCurrentTechnique = 0;
	
	return true;
}

bool CQuadrionEffect::RenderEffect(void(*renderCallback(void)))
{
//	m_pEffectHandle->SetTechnique(m_hCurrentTechnique);

	if(m_bReloadStateBlock)
	{
		if(m_hStateBlock != NULL)
			m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
	
		m_pEffectHandle->BeginParameterBlock();
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, m_effectParameterNames[i].c_str());
			if(hParam)
			{
				if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
				{
					if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_BOOL)
						m_pEffectHandle->SetBool(hParam, *(BOOL*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_INT)
						m_pEffectHandle->SetInt(hParam, *(INT*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_FLOAT)
						m_pEffectHandle->SetFloat(hParam, *(FLOAT*)(m_effectParameterValues[i]));
					else
						m_pEffectHandle->SetValue(hParam, m_effectParameterValues[i], effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]);
				}
			}
		}		
		m_hStateBlock = m_pEffectHandle->EndParameterBlock();
		m_bReloadStateBlock = false;
	}

	m_pEffectHandle->ApplyParameterBlock(m_hStateBlock);

	unsigned int nPasses;
	if(FAILED(m_pEffectHandle->Begin(&nPasses, 0)))
		return false;
	
	if(nPasses > 0)
		m_nPasses = nPasses;
	
	else
		return false;

	for(unsigned int i = 0; i < m_nPasses; ++i)
	{
		m_pEffectHandle->BeginPass(i);	
		
		renderCallback();
		
		m_pEffectHandle->EndPass();
	}	
	
	m_pEffectHandle->End();
//	m_hCurrentTechnique = 0;
	
	return true;	
}

bool CQuadrionEffect::RenderEffect(const cTextureRect& rect, const unsigned int& width, const unsigned int& height)
{
//	m_pEffectHandle->SetTechnique(m_hCurrentTechnique);

	if(m_bReloadStateBlock)
	{
		if(m_hStateBlock != NULL)
			m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
			
		m_pEffectHandle->BeginParameterBlock();
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, m_effectParameterNames[i].c_str());
			if(hParam)
			{
				if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
				{
					if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_BOOL)
						m_pEffectHandle->SetBool(hParam, *(BOOL*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_INT)
						m_pEffectHandle->SetInt(hParam, *(INT*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_FLOAT)
						m_pEffectHandle->SetFloat(hParam, *(FLOAT*)(m_effectParameterValues[i]));
					else
						m_pEffectHandle->SetValue(hParam, m_effectParameterValues[i], effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]);
				}
			}
		}		
		m_hStateBlock = m_pEffectHandle->EndParameterBlock();
		m_bReloadStateBlock = false;
	}

	m_pEffectHandle->ApplyParameterBlock(m_hStateBlock);

	unsigned int nPasses;
	if(FAILED(m_pEffectHandle->Begin(&nPasses, 0)))
		return false;
	
	if(nPasses > 0)
		m_nPasses = nPasses;
	
	else
		return false;

	for(unsigned int i = 0; i < m_nPasses; ++i)
	{
		m_pEffectHandle->BeginPass(i);	
		
		RenderFullscreenQuad(m_pRenderDevice, rect, width, height);
		
		m_pEffectHandle->EndPass();
	}	
	
	m_pEffectHandle->End();
//	m_hCurrentTechnique = 0;
	
	return true;	
}


bool CQuadrionEffect::RenderEffect(const cTextureRect& rect, const unsigned int& width, const unsigned int& height, const vec3f* V)
{
//	m_pEffectHandle->SetTechnique(m_hCurrentTechnique);

	if(m_bReloadStateBlock)
	{
		if(m_hStateBlock != NULL)
			m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
			
		m_pEffectHandle->BeginParameterBlock();
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, m_effectParameterNames[i].c_str());
			if(hParam)
			{
				if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
				{
					if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_BOOL)
						m_pEffectHandle->SetBool(hParam, *(BOOL*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_INT)
						m_pEffectHandle->SetInt(hParam, *(INT*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_FLOAT)
						m_pEffectHandle->SetFloat(hParam, *(FLOAT*)(m_effectParameterValues[i]));
					else
						m_pEffectHandle->SetValue(hParam, m_effectParameterValues[i], effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]);
				}
			}
		}		
		m_hStateBlock = m_pEffectHandle->EndParameterBlock();
		m_bReloadStateBlock = false;
	}

	m_pEffectHandle->ApplyParameterBlock(m_hStateBlock);

	unsigned int nPasses;
	if(FAILED(m_pEffectHandle->Begin(&nPasses, 0)))
		return false;
	
	if(nPasses > 0)
		m_nPasses = nPasses;
	
	else
		return false;

	for(unsigned int i = 0; i < m_nPasses; ++i)
	{
		m_pEffectHandle->BeginPass(i);	
		
		RenderFullscreenQuadFrustumCorners(m_pRenderDevice, rect, width, height, V);
		
		m_pEffectHandle->EndPass();
	}	
	
	m_pEffectHandle->End();
//	m_hCurrentTechnique = 0;
	
	return true;	
}

bool CQuadrionEffect::RenderEffect( const int& pass )
{

	if(m_bReloadStateBlock)
	{
		if(m_hStateBlock != NULL)
			m_pEffectHandle->DeleteParameterBlock(m_hStateBlock);
			
		m_pEffectHandle->BeginParameterBlock();
		for(unsigned int i = 0; i < m_nParameters; ++i)
		{
			D3DXHANDLE hParam = m_pEffectHandle->GetParameterByName(NULL, m_effectParameterNames[i].c_str());
			if(hParam)
			{
				if(m_pEffectHandle->IsParameterUsed(hParam, m_hCurrentTechnique))
				{
					if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_BOOL)
						m_pEffectHandle->SetBool(hParam, *(BOOL*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_INT)
						m_pEffectHandle->SetInt(hParam, *(INT*)(m_effectParameterValues[i]));
					else if(m_effectParameterTypes[i] == QEFFECT_VARIABLE_FLOAT)
						m_pEffectHandle->SetFloat(hParam, *(FLOAT*)(m_effectParameterValues[i]));
					else
						m_pEffectHandle->SetValue(hParam, m_effectParameterValues[i], effectVarSizes[m_effectParameterTypes[i]] * m_effectParameterCounts[i]);
				}
			}
		}		
		
		m_hStateBlock = m_pEffectHandle->EndParameterBlock();
		m_bReloadStateBlock = false;
	}

	m_pEffectHandle->ApplyParameterBlock(m_hStateBlock);

	unsigned int nPasses;
	if(FAILED(m_pEffectHandle->Begin(&nPasses, 0)))
		return false;


	m_pEffectHandle->BeginPass(pass);	
		
	return true;	
}

bool CQuadrionEffect::EndRender( const int& pass )
{	
	m_pEffectHandle->EndPass();
	m_pEffectHandle->End();
	
	return true;
}
