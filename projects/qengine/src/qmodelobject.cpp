#include "qmodelobject.h"


CModelObject::CModelObject(const unsigned int handle, const std::string& name, const std::string& path)
{
	m_fileName = path + name;
	m_bIsLoaded = false;
	m_bIsRenderable = false;
	m_bHasNormalmaps = false;
	m_handle = handle;
	
	m_effectHandle = QRENDER_INVALID_HANDLE;
	
	QMATH_MATRIX_LOADIDENTITY(m_modelPose);
	m_modelScale.set(1.0F, 1.0F, 1.0F);
	
	m_diffuseBindPoint = 0;
	m_normalmapBindPoint = 2;
}

CModelObject::~CModelObject()
{
	if(QRENDER_IS_VALID(m_effectHandle))
		g_pRender->UnloadEffect(m_effectHandle);
}

void CModelObject::BindDiffuseTexture( const int& texUnit )
{
	m_diffuseBindPoint = texUnit;
}

void CModelObject::BindNormalmapTexture( const int& texUnit )
{
	m_normalmapBindPoint = texUnit;
}


void CModelObject::RenderModel()
{

}

bool CModelObject::LoadEffect(const std::string& fxName, const std::string& fxPath)
{
	m_effectHandle = g_pRender->AddEffect(fxName, fxPath);
	if(QRENDER_IS_VALID(m_effectHandle))
		return true;
	
	return false;
}

void CModelObject::CreateFinalTransform(mat4& M)
{
	mat4 T, S, NT, TMP;
	
//	QMATH_MATRIX_LOADTRANSLATION(T, m_worldPos - m_modelCenter);
//	QMATH_MATRIX_LOADTRANSLATION(NT, m_modelCenter);
//	QMATH_MATRIX_LOADSCALE(S, m_modelScale);	
		
//	QMATH_MATRIX_MULTIPLY(T, NT, TMP);
	QMATH_MATRIX_MULTIPLY(TMP, m_modelPose, T);
	QMATH_MATRIX_LOADTRANSLATION(NT, vec3f(-m_modelCenter.x, -m_modelCenter.y, -m_modelCenter.z));
//	QMATH_MATRIX_MULTIPLY(T, S, TMP);
	QMATH_MATRIX_MULTIPLY(TMP, NT, NT);
	g_pRender->MulMatrix(QRENDER_MATRIX_MODEL, m_modelPose);
	
	QMATH_MATRIX_COPY(M, NT);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


CModelObjectInstance::CModelObjectInstance(const unsigned int handle, const std::string& name, const std::string& path) : CModelObject(handle, name, path)
{
	m_handle = handle;
	m_fileName = path + name;
	m_pRootModel = NULL;
}

CModelObjectInstance::~CModelObjectInstance()
{
	m_pRootModel = NULL;
}


void CModelObjectInstance::RenderModel()
{
	if(m_pRootModel)
		m_pRootModel->RenderModel();
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

CModelManager::CModelManager()
{

}

CModelManager::~CModelManager()
{
	std::map <std::string, std::vector<CModelObject*>, ltstr>::iterator iter;
	for(iter = m_modelMap.begin(); iter != m_modelMap.end(); iter++)
	{
		CModelObject* mdl = iter->second.at(0);
		delete mdl;
	}
	
	m_modelMap.clear();
}



int CModelManager::AddModel( const std::string& name, const std::string& path, bool loadNormalmaps )
{
	if(name.empty() || path.empty())
		return -1;
	
	
	std::string file_name = path + name;
	bool isEmpty = m_modelMap.empty();

	// Check for root existance //
	if(!isEmpty)
	{
		int size = m_modelMap[file_name].size();
		CModelObjectInstance* new_model = NULL;
	
		// Look for open spot //
		for(int i = 0; i < size; ++i)
		{
			if(!m_modelMap[file_name].at(i))
			{
				new_model = new CModelObjectInstance(i, name, path);
				m_modelMap[file_name][i] = new_model;
				return i;
			}
		}
		
		// Otherwise tack it on the end //

		new_model = new CModelObjectInstance(size, name, path);
		m_modelMap[file_name].push_back(new_model);
		return size;
	}
	
	// Otherwise make a new root node //
	CModelObject* new_base = NULL;
	if(name.find(".3ds") != std::string::npos || name.find(".3DS") != std::string::npos)
	{
		new_base = new c3DSModel(0, name, path);
		new_base->SetTexturePath(m_texturePath);
		new_base->SetFilePath( path );
		if( !new_base->LoadModel( loadNormalmaps ) )
		{
			if(new_base)
			{
				delete new_base; 
				new_base = NULL;
				return -1;
			}
		}
		
//		std::string fname = m_effectPath + "effect_modelinstance.fx";
//		if(!new_base->LoadEffect(fname))
//		{
//			delete new_base;
//			new_base = NULL;
//			return -1;
//		}
		
		m_modelMap[file_name].push_back(new_base);
		return 0;
	}
	
	if(name.find(".md3") != std::string::npos || name.find(".MD3") != std::string::npos)
	{
		new_base = new CMD3Mesh(0, name, path);
		if( !new_base->LoadModel( loadNormalmaps ) )
		{
			if(new_base)
			{
				delete new_base;
				new_base = NULL;
				return -1;
			}
		}
		
		std::string fname = m_effectPath + "effect_modelinstance.fx";
		if(!new_base->LoadEffect(fname))
		{
			delete new_base;
			new_base = NULL;
			return -1;
		}
		
		m_modelMap[file_name].push_back(new_base);
		return 0;
	}
	
	if(name.find(".qm3") != std::string::npos || name.find(".QM3") != std::string::npos)
	{
		new_base = new CMD3Model(0, name, path);
		if( !new_base->LoadModel( loadNormalmaps ) )
		{
			if(new_base)
			{
				delete new_base;
				new_base = NULL;
				return -1;
			}
		}

		if(!new_base->LoadEffect("fx/effect_modelinstance.fxo"))
		{
			delete new_base;
			new_base = NULL;
			return -1;
		}

		m_modelMap[file_name].push_back(new_base);
		return 0;
	}
	
	// Fail //
	return -1;
}



CModelObject* CModelManager::GetModel(const std::string& name, const std::string& path, const int& handle)
{	
	std::string concat = path + name;
	if(handle < m_modelMap[concat].size())
		return m_modelMap[concat][handle];

	return NULL;
}




void CModelManager::RemoveModel(const std::string& name, const std::string& path, const int& handle)
{
	std::string concat = path + name;
	
	// Not the base model //
	if(handle != 0)
	{
		if(m_modelMap[concat][handle])
		{
			delete m_modelMap[concat][handle];
			m_modelMap[concat][handle] = NULL;
		}
	}
	
	// Is the base model //
	else
		m_modelMap[concat].clear();	
}



void CModelManager::RenderVisibleModelsBSP()
{
	CQuadrionEffect* model_effect;
	mat4 W, PW;
	g_pRender->GetMatrix(QRENDER_MATRIX_MODEL, PW);
	
	std::map <std::string, std::vector<CModelObject*>, ltstr>::iterator iter;
	for(iter = m_modelMap.begin(); iter != m_modelMap.end(); iter++)
	{
		CModelObject* mdl = iter->second.at(0);
		mdl->CreateFinalTransform(W);
		model_effect = g_pRender->GetEffect(mdl->GetEffectHandle());
		model_effect->BeginEffect("instanced_model_technique");
		
		unsigned int mat = QEFFECT_MATRIX_WORLDVIEWPROJECTION;
		g_pRender->MulMatrix(QRENDER_MATRIX_MODEL, W);
		model_effect->UploadParameters("g_matWorldViewProj", QEFFECT_VARIABLE_STATE_MATRIX, 1, &mat);
		//model_effect->UploadStateMatrix(QEFFECT_MATRIX_WORLDVIEWPROJECTION);
		model_effect->RenderEffect(0);
		
		mdl->BindDiffuseTexture( m_diffuseBindPoint );
		mdl->BindDiffuseTexture( m_normalmapBindPoint );
		mdl->RenderModel();
		
		model_effect->EndRender(0);
		model_effect->EndEffect();
	}
	
	g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, PW);
}


void CModelManager::RenderVisibleModelsBSPCallback(void* self)
{
	CModelManager* myself = (CModelManager*)self;
	myself->RenderVisibleModelsBSP();
}
