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

	m_modelInstanceMatrices = new float[16 * MAX_MODEL_INSTANCES];
	m_nModelInstances = 1;
}

CModelObject::~CModelObject()
{
	if(QRENDER_IS_VALID(m_effectHandle))
		g_pRender->UnloadEffect(m_effectHandle);

	int i = 0;
	for(i = 0; i < m_vertexBufferHandles.size(); ++i)
	{
		if(QRENDER_IS_VALID(m_vertexBufferHandles[i]))
			g_pRender->UnloadInstancedVertexBuffer(m_vertexBufferHandles[i]);
	}
	m_vertexBufferHandles.clear();

	for(i = 0; i < m_indexBufferHandles.size(); ++i)
	{
		if(QRENDER_IS_VALID(m_indexBufferHandles[i]))
			g_pRender->UnloadIndexBuffer(m_indexBufferHandles[i]);
	}
	m_indexBufferHandles.clear();

	if(m_modelInstanceMatrices)
	{
		delete[] m_modelInstanceMatrices;
		m_modelInstanceMatrices = 0;
	}
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

	QMATH_MATRIX_LOADTRANSLATION(T, vec3f(-m_modelCenter));
	QMATH_MATRIX_LOADSCALE(S, vec3f(m_modelScale));
	QMATH_MATRIX_MULTIPLY(S, T, T);
	QMATH_MATRIX_MULTIPLY(m_modelPose, T, m_modelPose); 

//	g_pRender->MulMatrix(QRENDER_MATRIX_MODEL, m_modelPose);
//
//	QMATH_MATRIX_COPY(M, m_modelPose);

/*
	CQuadrionInstancedVertexBuffer* vb = NULL;
	for(int i = 0; i < m_vertexBufferHandles.size(); ++i)
	{
		vb = g_pRender->GetInstancedVertexBuffer(m_vertexBufferHandles[i]);
		QMATH_MATRIX_TRANSPOSE(m_modelPose);
		vb->UpdateInstanceBuffer(m_modelPose, 1);
		QMATH_MATRIX_TRANSPOSE(m_modelPose);
	}
*/
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


CModelObjectInstance::CModelObjectInstance(const unsigned int handle, const std::string& name, const std::string& path) : CModelObject(handle, name, path)
{
	m_handle = handle;
	m_fileName = path + name;
	m_pRootModel = NULL;
	m_bIsActive = true;

	QMATH_MATRIX_LOADIDENTITY(m_orientation);
}

CModelObjectInstance::~CModelObjectInstance()
{
	m_pRootModel = NULL;
	m_bIsActive = false;
}


void CModelObjectInstance::RenderModel()
{
//	if(m_pRootModel)
//		m_pRootModel->RenderModel();
}

void CModelObjectInstance::SetModelOrientation(const mat4& m)
{
	QMATH_MATRIX_COPY(m_orientation, m);
}

void CModelObjectInstance::GetModelOrientation(mat4& out)
{
	QMATH_MATRIX_COPY(out, m_orientation);
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
				m_modelMap[file_name][0]->m_nModelInstances++;
				return i;
			}
		}
		
		// Otherwise tack it on the end //
		new_model = new CModelObjectInstance(size, name, path);
		m_modelMap[file_name].push_back(new_model);
		m_modelMap[file_name][0]->m_nModelInstances++;
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

		// If the model was created, go ahead and allocate the instance buffers
		// for the VBO.
		if(new_base->m_vertexBufferHandles.size() > 0)
		{
			mat4 id;
			QMATH_MATRIX_LOADIDENTITY(id);
			memcpy(new_base->m_modelInstanceMatrices, id, sizeof(float) * 16);

			for(int i = 0; i < new_base->m_vertexBufferHandles.size(); ++i)
			{
				CQuadrionInstancedVertexBuffer *vb = g_pRender->GetInstancedVertexBuffer(new_base->m_vertexBufferHandles[i]);
				vb->CreateInstanceBuffer(new_base->m_modelInstanceMatrices, MAX_MODEL_INSTANCES);
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

void CModelManager::UpdateModelOrientation(const std::string& name, const std::string& path, int handle, const mat4& newPose)
{
	std::string concat = path + name;
	if(handle < m_modelMap[concat].size())
	{
		if(handle == 0)
		{
			CModelObject* root = m_modelMap[concat][0];
			root->SetModelOrientation(newPose);
		}

		else
		{
			CModelObjectInstance* inst = (CModelObjectInstance*)(m_modelMap[concat][handle]);
			inst->SetModelOrientation(newPose);
		}
	}
}

void CModelManager::PushInstances(const std::string& name, const std::string& path)
{
	std::string concat = path + name;
	CModelObject* root = m_modelMap[concat][0];
	memcpy(root->m_modelInstanceMatrices, root->m_modelPose, sizeof(float) * 16);
	for(int i = 1; i < m_modelMap[concat].size(); ++i)
	{
		CModelObjectInstance* inst = (CModelObjectInstance*)(m_modelMap[concat][i]);
		memcpy((void*)&(root->m_modelInstanceMatrices[i * 16]), inst->m_orientation, sizeof(float) * 16);
	}

	CQuadrionInstancedVertexBuffer* vb = NULL;
	for(int i = 0; i < root->m_vertexBufferHandles.size(); ++i)
	{
		vb = g_pRender->GetInstancedVertexBuffer(root->m_vertexBufferHandles[i]);
		vb->UpdateInstanceBuffer(root->m_modelInstanceMatrices, root->m_nModelInstances);
	}
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
