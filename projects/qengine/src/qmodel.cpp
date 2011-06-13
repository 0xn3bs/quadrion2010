#include "qmodel.h"
#include "qmd3.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

CQuadrionModel::CQuadrionModel(const unsigned int handle, const std::string& name, const std::string& path) 
: m_sName(name), m_sPath(path)
{
	m_eModelType = MODEL_TYPE_UNKNOWN;
	QMATH_MATRIX_LOADIDENTITY(m_m4PreviousTransform);
	m_quatRotation.set(0.0f, 0.0f, 0.0f, 0.0f);
	m_v3Position.set(0.0f, 0.0f, 0.0f);
	m_v3Scale.set(1.0f, 1.0f, 1.0f);

	m_pBaseTexture = 0;
	m_pLightMapTexture = 0;
}

CQuadrionModel::~CQuadrionModel()
{
}

void CQuadrionModel::SetRender(void* render)
{ 
	m_pRenderer = (CQuadrionRender*)render;
}

void CQuadrionModel::SetBaseTexture(CQuadrionTextureObject* pTex)
{
	m_pBaseTexture = pTex;
}

void CQuadrionModel::SetLightmapTexture(CQuadrionTextureObject* pTex)
{
	m_pLightMapTexture = pTex;
}

void CQuadrionModel::SetModelType(const EModelType type)
{
	m_eModelType = type;
}

CQuadrionTextureObject* CQuadrionModel::GetBaseTexture()
{
	return m_pBaseTexture;
}

CQuadrionTextureObject* CQuadrionModel::GetLightmapTexture()
{
	return m_pLightMapTexture;
}

void CQuadrionModel::RenderCallback(LPVOID self)
{
	CQuadrionModel* myself = (CQuadrionModel*)self;
	myself->Render();
}

void CQuadrionModel::SetPosition(vec3f &pos)
{
	m_v3Position = vec3f(pos);
}

void CQuadrionModel::SetRotation(vec3f &axis, float angle)
{
	QMATH_QUATERNION_ROTATE(m_quatRotation, angle, axis);
};

const quat CQuadrionModel::GetRotation()
{
	return m_quatRotation;
}

vec3f CQuadrionModel::GetPosition()
{
	return m_v3Position;
}

CQuadrionModelManager::CQuadrionModelManager(CQuadrionRender* render)
{
	m_vModelPool = std::vector<CMD3Mesh*>();
	m_pRenderer = render;
	m_iHandleCount = 0;
	m_pDeltaTimer = new CTimer();
	m_pDeltaTimer->Stop();
	m_fLastTime = 0.0f;
}

CQuadrionModelManager::~CQuadrionModelManager()
{
	for(unsigned int i = 0; i < m_vModelPool.size(); ++i)
	{
		if(m_vModelPool[i])
		{
			delete m_vModelPool[i];
			m_vModelPool[i] = NULL;
		}
	}

	m_vModelPool.clear();
	m_vHandles.clear();
}

void CQuadrionModelManager::SetRender(CQuadrionRender* render)
{
	m_pRenderer = render;
}

// void CQuadrionModelManager::Render()
// {
// 	for(unsigned int i = 0; i < m_vModelPool.size(); ++i)
// 		m_vModelPool[i]->Render();
// }

void CQuadrionModelManager::Update()
{
// 	if(!m_pDeltaTimer->IsRunning)
// 	{
// 		m_fDeltaTime = 0.0f;
// 		m_fLastTime = 0.0f;
// 	}
// 	else
// 	{
// 		m_fDeltaTime = m_pDeltaTimer->GetElapsedMilliSec();
// 		m_pDeltaTimer->Stop();
// 	}
// 
// 	m_pDeltaTimer->Start();
// 
// 	for(unsigned int i = 0; i < m_vModelPool.size(); ++i)
// 		m_vModelPool[i]->Update();
}

void CQuadrionModelManager::PreRender(CQuadrionEffect* effect)
{
// 	for(unsigned int i = 0; i < m_vModelPool.size(); ++i)
// 	{
// 		m_vModelPool[i]->PreRender();
// 		effect->UploadParameters("matrix_modelviewprojection", QEFFECT_VARIABLE_STATE_MATRIX, 1, (LPVOID)&QEFFECT_MATRIX_WORLDVIEWPROJECTION);
// 		effect->RenderEffect((LPVOID)m_vModelPool[i], &CQuadrionModel::RenderCallback);
// 	}
}

// void CQuadrionModelManager::DebugRenderTags()
// {
// 	for(unsigned int i = 0; i < m_vModelPool.size(); ++i)
// 		m_vModelPool[i]->DebugRenderTags();
// }

// int CQuadrionModelManager::AddModel(const std::string& name, const std::string& path)
// {
// 	if(name.size() <= 0 || path.size() <= 0)
// 		return -1;
// 
// 	CMD3Model* pmd3Model = new CMD3Model(m_iHandleCount, name, path);
// 	pmd3Model->Initialize(m_pRenderer);
// 
// 	if(pmd3Model->LoadModel())
// 	{
// 		pmd3Model->SetModelType(MODEL_TYPE_MD3);
// 		m_vModelPool.push_back(pmd3Model);
// 		m_vHandles.push_back(m_iHandleCount);
// 		return m_iHandleCount++;
// 	}
// 	else
// 	{
// 		delete pmd3Model;
// 		pmd3Model = NULL;
// 	}
// 
// 	return -1;
// }

CQuadrionModel* CQuadrionModelManager::GetModel(unsigned int handle)
{
	//for(unsigned int i = 0; m_vHandles.size(); ++i)
 //	{
 //		if(m_vHandles[i] == handle)
 //			return m_vModelPool[i];
 //	}
 //
 	return NULL;
}

CQuadrionModelAssembler::CQuadrionModelAssembler()
{
}

CQuadrionModelAssembler::CQuadrionModelAssembler(std::string path)
{
	Assemble(path);
}

CQuadrionModelAssembler::~CQuadrionModelAssembler()
{

}

const int CQuadrionModelAssembler::Assemble(std::string path)
{
	CXML* pXMLFile = new CXML(path);

	if(pXMLFile->IsValidFile())
	{
		//	Get version information
		const CXMLNode* pVersionInfoNode = pXMLFile->GetNode("MODEL_XML_ASSEMBLER");
		std::string sVal = pVersionInfoNode->GetAttributeValue("major");
		unsigned int uiMajor = atoi(sVal.c_str());
		sVal = pVersionInfoNode->GetAttributeValue("minor");
		unsigned int uiMinor = atoi(sVal.c_str());
		sVal = pVersionInfoNode->GetAttributeValue("bugfix");
		unsigned int uiBugfix = atoi(sVal.c_str());

		if(uiMajor <= MODEL_XML_ASSEMBLER_MAJOR 
			&& uiMinor <= MODEL_XML_ASSEMBLER_MINOR 
			&& uiBugfix <= MODEL_XML_ASSEMBLER_BUGFIX)
		{
			const CXMLNode* pModelNode = pXMLFile->GetNode("model");
			unsigned int uiNumMeshes = pModelNode->GetNumNodes("mesh");
			for(unsigned int i = 0; i < uiNumMeshes; ++i)
			{
				const CXMLNode* pMeshNode = pModelNode->GetNode("mesh", i);
			}
		}
		else
		{
			//	Output a warning or error for possible incompatibilities.
		}
	}

	return 0;
}

//void CModelManager::precache(const char *path)
//{
//	precache(std::string(path));
//}
//
//void CModelManager::precache(std::string &path)
//{
//	//	Is this model already loaded? If so, don't reload.
//	//	TODO: Write a routine to reload models.
//	for(int i = 0; i < m_vModels.size(); ++i)
//		if(path.compare(m_vModels[i]->strPath)==0)
//			return;
//
//	//	Grab the extension.
//	//	TODO: Trim excess whitespace and trailing slashes (if any).
//	
//	//  Find delimiter
//	
//	size_t delimPos = path.find_last_of('.');
//
//	//	We don't know what type of file this is unless we parse it..
//	//	TODO: Worry about that later. Checking the extension is sufficient for now.
//
//	if(delimPos == std::string::npos)
//		return;
//	
//	std::string strExtension = path.substr(delimPos, path.size() - delimPos);
//
//	//	Convert the extension to lower case characters before comparing.
//	std::transform(strExtension.begin(), strExtension.end(), strExtension.begin(), (int(*)(int))std::tolower);
//
//	if(strExtension.compare(".3ds") == 0)
//	{
//		c3DSModel *tempModel = new c3DSModel(path.c_str());
//		C3DSModelHandle *tempModelHandle = new C3DSModelHandle(tempModel);
//
//		m_vModels.push_back(tempModelHandle);
//	}
//}
//
//void CModelManager::uncache(const char* path)
//{
//	for(int i = 0; i < m_vModels.size(); ++i)
//	{
//		if(strcmp(m_vModels[i]->strPath.c_str(), path) == 0)
//		{
//			delete m_vModels[i];
//			m_vModels.erase(m_vModels.begin()+i);
//		}
//	}
//}
//
//void CModelManager::uncache(CModel* model)
//{
//	for(int i = 0; i < m_vModels.size(); ++i)
//	{
//		if(model == m_vModels[i])
//		{
//			delete m_vModels[i];
//			m_vModels.erase(m_vModels.begin()+i);	
//		}
//	}
//}
//
//CModelManager::~CModelManager()
//{
//	if(!m_vModels.empty())
//	{
//		for(int i = 0; i < m_vModels.size(); ++i)
//			delete m_vModels[i];
//
//		m_vModels.clear();
//	}
//}
/*

CModel::CModel()
{
	QMATH_MATRIX_LOADIDENTITY(modelMatrix);
	QMATH_MATRIX_LOADIDENTITY(currentRotation);
	modelPos.set( 0.0, 0.0, 0.0 );
}

CModel::~CModel()
{
	
}

void CModel::init()
{
//	effectID = g_pRender->addEffect( baseDir("projects/FPS/fx/Phong.fx" ));
}



void CModel::rotate( const vec3f axis, const float angInRad )
{
//	mat4 rollMatrix;
//	mat_loadRollFromVec3( rollMatrix, axis, angInRad );
//	mat_mulMat( rollMatrix, modelMatrix, modelMatrix );
}

void CModel::scale( const vec3f axis )
{
	modelScale.set(axis);
}

void CModel::getFinalTransform( mat4& m, const int mesh )
{
	vec3f minbb, maxbb;
	getBoundingBox( minbb, maxbb );
	vec3f center = ( maxbb - minbb );
	center = minbb + ( center / 2.0F );

	mat4 T, S, R, NT, ID, TC;
	QMATH_MATRIX_COPY(R, modelMatrix);

	vec3f t(modelPos.x - center.x, modelPos.y - center.y, modelPos.z - center.z);
	QMATH_MATRIX_LOADTRANSLATION(T, t);
	
	vec3f s(modelScale.x, modelScale.y, modelScale.z);
	QMATH_MATRIX_LOADSCALE(S, s);
	
	vec3f nt(-center.x, -center.y, -center.z);
	QMATH_MATRIX_LOADTRANSLATION(NT, nt);
	
	vec3f tc(center.x, center.y, center.z);
	QMATH_MATRIX_LOADTRANSLATION(TC, tc);
	
	QMATH_MATRIX_LOADIDENTITY(ID);
	
	// PRE //
	QMATH_MATRIX_MULTIPLY(NT, ID, ID);
	QMATH_MATRIX_MULTIPLY(S, ID, ID);
	QMATH_MATRIX_MULTIPLY(R, ID, ID);
	QMATH_MATRIX_MULTIPLY(TC, ID, ID);
	QMATH_MATRIX_MULTIPLY(T, ID, ID);
	QMATH_MATRIX_COPY(m, ID);
}

void CModel::renderModel()
{
	g_pRender->changeVertexFormat(vertexFormatHandle);
	g_pRender->ChangeCullMode(QRENDER_CULL_NONE);
	
	mat4 T, TC, R, S, NT;
	
	// Render all meshes //
	for( int i = 0; i < mdlData.meshCount; ++i )
	{
		// sanity check
		if( mdlData.meshes[i].vertCount <= 0 || mdlData.meshes[i].triCount <= 0 )
			continue;
	
		g_pRender->changeVertexBuffer(meshRenderHandles[i].vboRef);
		g_pRender->changeIndexBuffer(meshRenderHandles[i].iboRef);
		
		// remember current model matrix to be restored after transform //
		mat4 PRE;
		g_pRender->getMatrix(QRENDER_MATRIX_MODEL, PRE);
			
		// obtain center point of model (approx)
		vec3f minbb, maxbb;
		getMeshBoundingBox( i, minbb, maxbb );
		vec3f center = ( maxbb - minbb );
		center = minbb + ( center / 2.0F );

		QMATH_MATRIX_COPY(R, modelMatrix);
	
		// desired position translation //
		vec3f t(modelPos.x - center.x, modelPos.y - center.y, modelPos.z - center.z);
		QMATH_MATRIX_LOADTRANSLATION(T, t);
	
		// load scaling matrix //
		vec3f s(modelScale.x, modelScale.y, modelScale.z);
		QMATH_MATRIX_LOADSCALE(S, s);
	
		// load translation to world origin //
		vec3f nt(-center.x, -center.y, -center.z);
		QMATH_MATRIX_LOADTRANSLATION(NT, nt);
	
		// load translation back to desired position //
		vec3f tc(center.x, center.y, center.z);
		QMATH_MATRIX_LOADTRANSLATION(TC, tc);
		
		// compute final transform //
		g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, T );			// desired translation
		g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, TC );		// translation to center
		g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, R );			// rotate
		g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, S );			// scale
		g_pRender->MulMatrix( QRENDER_MATRIX_MODEL, NT );		// reverse translation
		g_pRender->setEffectStateMatrix(QRENDER_MATRIX_MODELVIEW, QRENDER_MATRIX_IDENTITY, "matWorldView");
		g_pRender->setEffectStateMatrix(QRENDER_MATRIX_MODELVIEWPROJECTION, QRENDER_MATRIX_IDENTITY, "matWorldViewProjection");
		vec3f ambient((float)r / 255.0, (float)g / 255.0, (float)b / 255.0);
		ambient.set(0.5, 0.5, 0.5);
		g_pRender->setArbitraryValue(&ambient, 12, "ambient");
		
		// Loop through material groups //
		for( int j = 0; j < mdlData.meshes[i].groupCount; ++j )
		{
			if( mdlData.meshes[i].groups[j].size <= 0 && mdlData.meshes[i].groupCount > 1 )
				continue;
			
			int groupRef = mdlData.meshes[i].groups[j].mat;
			
			bool hasTextures = true;
			if(mdlData.meshes[i].groups[j].mat > 0)
			{
				if( textureHandleList[groupRef].textureRef == QRENDER_INVALID_TEXTURE )
					hasTextures = false;
			}

			int start = mdlData.meshes[i].groups[j].start;
				
			if(hasTextures)
				g_pRender->changeTexture( textureHandleList[groupRef].textureRef, 0 );
			UINT tm = start * 3;
			UINT cr = 0;
			g_pRender->RenderIndexedList(QRENDER_PRIM_TRIANGLES, tm, cr, cr, cr);
		}
		
		g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, PRE);
	}

	g_pRender->ChangeCullMode(QRENDER_CULL_DEFAULT);
	g_pRender->changeVertexFormat(QRENDER_INVALID_VERTEX_FORMAT);
	g_pRender->changeVertexBuffer(QRENDER_INVALID_VERTEX_BUFFER);
	g_pRender->changeIndexBuffer(QRENDER_INVALID_INDEX_BUFFER);
	g_pRender->changeTexture(QRENDER_INVALID_TEXTURE, 0);
}


CModel *cModelMgr::loadModel(char *file)
{
	// Loop through all currently loaded models to see if its already loaded
	for(int i = 0; i < models.size(); i++)
	{
		// Found the model, return its ptr
		if(strcmp(models[i]->getFileName().c_str(), file) == 0)
			return models[i];
	}

	CModel *newModel = new CModel();
	newModel->loadModel(file, false);
	newModel->init();
	newModel->setPos(vec3f(0,0,0));
	unsigned char color[3] = { 0xFF, 0xFF, 0xFF };
	newModel->setAmbientColor(color);
	newModel->scale(vec3f(0.03, 0.03, 0.03));
	newModel->id = nextID;

	models.push_back(newModel);

	nextID++;

	return newModel;
}

CModel *cModelMgr::getModel(int id)
{
	for(int i = 0; i < models.size(); i++)
	{
		// Found the model, return its ptr
		if(models[i]->id == id)
			return models[i];
	}

	// If it gets here, it couldn't find the model
	return 0;
}

CModel *cModelMgr::getModel(char *file)
{
	for(int i = 0; i < models.size(); i++)
	{
		// Found the model, return its ptr
		if(strcmp(models[i]->getFileName().c_str(), file) == 0)
			return models[i];
	}

	// If it gets here, it couldn't find the model
	return 0;
}
*/