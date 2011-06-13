#include "qmd3.h"
#include <iostream>
#include <sstream>

CMD3Surface::CMD3Surface()
{
	m_iRefId = 0;
}

CMD3Surface::~CMD3Surface()
{
	for(unsigned int i = 0; i < m_vVerts.size(); ++i)
		if(m_vVerts[i])
			delete m_vVerts[i];

	m_vVerts.clear();
	
	//	Are we deleting the last frame? If so destroy all shared objects too.
	if(m_iRefId == 0)
	{
		for(unsigned int i = 0; i < m_vTriangles.size(); ++i)
			if(m_vTriangles[i])
			{
				delete m_vTriangles[i];
				m_vTriangles[i] = NULL;
			}
		
		m_vTriangles.clear();
		
		for(unsigned int i = 0; i < m_vTexCoords.size(); ++i)
			if(m_vTexCoords[i])
			{
				delete m_vTexCoords[i];
				m_vTexCoords[i] = NULL;
			}
		
		m_vTexCoords.clear();
	}
}

CMD3Frame::CMD3Frame()
{
	
}

CMD3Frame::~CMD3Frame()
{
	for(unsigned int i = 0; i < m_vTags.size(); ++i)
	{
		if(m_vTags[i])
		{
			delete m_vTags[i];
			m_vTags[i] = NULL;
		}
	}
	
	m_vTags.clear();
	
	for(unsigned int i = 0; i < m_vSurfaces.size(); ++i)
	{
		if(m_vSurfaces[i])
		{
			delete m_vSurfaces[i];
			m_vSurfaces[i] = NULL;
		}
	}
	
	m_vSurfaces.clear();
}

void CMD3Frame::AddTag(SMD3Tag* pTag)
{
	if(pTag)
		m_vTags.push_back(pTag);
}

void CMD3Frame::AddSurface(CMD3Surface* pSurface)
{
	if(pSurface)
		m_vSurfaces.push_back(pSurface);
}

CMD3Surface* CMD3Frame::GetSurface(unsigned int i)
{
	if(i >= m_vSurfaces.size())
		return NULL;
	
	return m_vSurfaces[i];
}

CMD3Mesh::CMD3Mesh(const unsigned int handle, const std::string& name, const std::string& path)
: CModelObject(handle, name, path)
{
	m_modelCenter.set(0.0f, 0.0f, 0.0f);
	m_worldPos.set(0.0f, 0.0f, 0.0f);
	m_modelScale.set(1.0f, 1.0f, 1.0f);
	QMATH_MATRIX_LOADIDENTITY(m_modelPose);
	
	m_iCurrentFrame = 0.0f;
	m_iTexRef = -1;
	
	m_sPath = path;
}

CMD3Mesh::~CMD3Mesh()
{
	for(unsigned int i = 0; i < m_vFrames.size(); ++i)
	{
		delete m_vFrames[i];
		m_vFrames[i] = NULL;
	}
	
	m_vFrames.clear();
}

bool CMD3Mesh::LoadModel()
{
	std::ifstream file((m_fileName).c_str(), std::ios::binary);
	if(!file.is_open())
		return false;
		
	file.seekg(0);
		
	SMD3Header *pHeader = new SMD3Header;
	
	file.read((char*)pHeader, sizeof(SMD3Header));
	
	if(pHeader->Ident != MD3_IDENT)
	{
		delete pHeader;
		return false;
	}
	
	file.seekg(pHeader->OffsetFrames);
		
	for(unsigned int i = 0; i < pHeader->NumFrames; ++i)
	{
		CMD3Frame *pFrame = new CMD3Frame;
		
		//	Read in frame data.
		file.read((char*)pFrame, sizeof(SMD3Frame));
		m_vFrames.push_back(pFrame);
	}
	
	file.seekg(pHeader->OffsetTags);
	
	for(unsigned int i = 0; i < pHeader->NumFrames; ++i)
	{
		for(unsigned int j = 0; j < pHeader->NumTags; ++j)
		{
			SMD3Tag *pTag = new SMD3Tag;
			file.read((char*)pTag, sizeof(SMD3Tag));
			m_vFrames[i]->AddTag(pTag);
		}
	}
	
	file.seekg(pHeader->OffsetSurfaces);
	
 	for(unsigned int i = 0; i < pHeader->NumSurfaces; ++i)
 	{
 		SMD3Surface *pMD3Surface = new SMD3Surface;
 		
 		int surfaceStart = file.tellg();
 		
 		file.read((char*)pMD3Surface, sizeof(SMD3Surface));
 		
 		//	Not a valid surface? Skip.
 		if(pMD3Surface->Ident != MD3_IDENT)
 		{
 			delete pMD3Surface;
 			continue;
 		}
 		
  		for(unsigned int j = 0; j < pHeader->NumFrames; ++j)
  		{
  			CMD3Surface* pSurface = new CMD3Surface;
  			pSurface->m_iRefId = pHeader->NumFrames - j - 1;
  			m_vFrames[j]->AddSurface(pSurface);
  		}
  		
		//	Read in triangles for this surface.
		file.seekg(surfaceStart + pMD3Surface->OffsetTriangles);
		for(unsigned int j = 0; j < pMD3Surface->NumTriangles; ++j)
		{
			SMD3Triangle *pTriangle = new SMD3Triangle;	
			file.read((char*)pTriangle, sizeof(SMD3Triangle));
			for(unsigned int k = 0; k < pHeader->NumFrames; ++k)
				m_vFrames[k]->GetSurface(i)->m_vTriangles.push_back(pTriangle);
		}
		
		//	Read in texture coordinates for this surface.
		file.seekg(surfaceStart + pMD3Surface->OffsetST);
		for(unsigned int j = 0; j < pMD3Surface->NumVerts; ++j)
		{
			SMD3TexCoord *pTexCoord = new SMD3TexCoord;
			file.read((char*)pTexCoord, sizeof(SMD3TexCoord));
			for(unsigned int k = 0; k < pHeader->NumFrames; ++k)
				m_vFrames[k]->GetSurface(i)->m_vTexCoords.push_back(pTexCoord);
		}
		
 		//	Read in vertex data for this surface
 		file.seekg(surfaceStart + pMD3Surface->OffsetXYZNormal);
 		for(unsigned int j = 0; j < pMD3Surface->NumFrames; ++j)
 		{
 			for(unsigned int k = 0; k < pMD3Surface->NumVerts; ++k)
 			{
 				SMD3Vertex *pVertex = new SMD3Vertex;
 				file.read((char*)pVertex, sizeof(SMD3Vertex));
 				m_vFrames[j]->GetSurface(i)->m_vVerts.push_back(pVertex);
 			}
 		}
 		
 		file.seekg(pHeader->OffsetSurfaces + pMD3Surface->OffsetEnd);
 	}
 	
	delete pHeader;
	
	//	Precache
	SQuadrionVertexDescriptor	desc;
	CQuadrionVertexBuffer*		vbo	= NULL;
	CQuadrionIndexBuffer*		ibo	= NULL;

	desc.pool	  = QVERTEXBUFFER_MEMORY_STATIC;
	desc.usage[0] = QVERTEXFORMAT_USAGE_POSITION;
	desc.size[0]  = QVERTEXFORMAT_SIZE_FLOAT3;

	desc.usage[1] = QVERTEXFORMAT_USAGE_NORMAL;
	desc.size[1]  = QVERTEXFORMAT_SIZE_FLOAT3;

	desc.usage[2] = QVERTEXFORMAT_USAGE_TANGENT;
	desc.size[2]  = QVERTEXFORMAT_SIZE_FLOAT3;

	desc.usage[3] = QVERTEXFORMAT_USAGE_TEXCOORD;
	desc.size[3]  = QVERTEXFORMAT_SIZE_FLOAT2;

	desc.usage[4] = QVERTEXFORMAT_USAGE_END;
	
	for(unsigned int i = 0; i < m_vFrames.size(); ++i)
	{
		CMD3Frame* pFrame = m_vFrames[i];
		
		for(unsigned int j = 0; j < pFrame->m_vSurfaces.size(); ++j)
		{
			CMD3Surface* pSurface = pFrame->m_vSurfaces[j];
			
			unsigned int numVerts = pSurface->m_vVerts.size();
			
			SMD3VertexFormat* pVerts = new SMD3VertexFormat[numVerts];
			
			for(unsigned int k = 0; k < numVerts; ++k)
			{
				SMD3Vertex* pVert = pSurface->m_vVerts[k];
				
				pVerts[k].x		= (float)pVert->Position[0] * MD3_XYZ_SCALE * 0.03f;
				pVerts[k].y		= (float)pVert->Position[2] * MD3_XYZ_SCALE * 0.03f;
				pVerts[k].z		= -(float)pVert->Position[1] * MD3_XYZ_SCALE * 0.03f;
				pVerts[k].nx	= pVert->Normal[0] *= (2*QMATH_PI)/255;
				pVerts[k].ny	= pVert->Normal[1] *= (2*QMATH_PI)/255;
				pVerts[k].vx	= 0;
				pVerts[k].vy	= 0;
				pVerts[k].vz	= 0;
				pVerts[k].u		= pSurface->m_vTexCoords[k]->ST[0];
				pVerts[k].v		= pSurface->m_vTexCoords[k]->ST[1];
			}
			
			pSurface->m_iVboHandle = g_pRender->AddVertexBuffer();
			
			vbo = g_pRender->GetVertexBuffer(pSurface->m_iVboHandle);
			vbo->CreateVertexBuffer(pVerts, desc, numVerts, FALSE);
			delete[] pVerts;
			pVerts = NULL;
			
			//	A surface shares the same index buffer regardless of it's frame #.
			if(pSurface->m_iRefId == m_vFrames.size()-1)
			{
				unsigned int numTriangles = pSurface->m_vTriangles.size();
				
				unsigned short* pIndices = new unsigned short[numTriangles * 3];
				
				for(unsigned int j = 0; j < numTriangles; j++)
				{
					SMD3Triangle* pTriangle = pSurface->m_vTriangles[j];
					pIndices[j * 3]		= pTriangle->Indexes[0];
					pIndices[j * 3 + 1] = pTriangle->Indexes[1];
					pIndices[j * 3 + 2] = pTriangle->Indexes[2];
				}
				
				pSurface->m_iIboHandle = g_pRender->AddIndexBuffer();
				ibo = g_pRender->GetIndexBuffer(pSurface->m_iIboHandle);
				
				ibo->CreateIndexBuffer(QINDEXBUFFER_MEMORY_STATIC, 
									   QINDEXBUFFER_SIZE_USHORT,
									   numTriangles * 3, 
									   pIndices);

				delete[] pIndices;
				pIndices = NULL;
			}
			else
				pSurface->m_iIboHandle = m_vFrames[0]->m_vSurfaces[j]->m_iIboHandle;
		}
	}
	
	unsigned int tex_flags = QTEXTURE_FILTER_TRILINEAR;
	m_iTexRef = g_pRender->AddTextureObject(tex_flags, m_sPath + std::string("homer.tga"));
	
	m_bIsLoaded = true;
	m_bIsRenderable = true;

	return true;
}

void CMD3Mesh::RenderModel()
{
	CMD3Frame* m_pFrame = m_vFrames[m_iCurrentFrame];
	CQuadrionTextureObject* pTexture = g_pRender->GetTextureObject(m_iTexRef);
	
	mat4 W, PW;
	g_pRender->GetMatrix(QRENDER_MATRIX_MODEL, PW);
	
	CreateFinalTransform(W);
	g_pRender->MulMatrix(QRENDER_MATRIX_MODEL, W);
	
	for(unsigned int i = 0; i < m_pFrame->m_vSurfaces.size(); i++)
	{
		g_pRender->ChangeCullMode(QRENDER_CULL_NONE);

		CQuadrionVertexBuffer* curVBO = NULL;
		CQuadrionIndexBuffer* curIBO = NULL;

		curVBO = g_pRender->GetVertexBuffer(m_pFrame->m_vSurfaces[i]->m_iVboHandle);
		curVBO->BindBuffer();

		curIBO = g_pRender->GetIndexBuffer(m_pFrame->m_vSurfaces[i]->m_iIboHandle);
		curIBO->BindBuffer();

		if(pTexture)
			pTexture->BindTexture(0);
		
		g_pRender->RenderIndexedList(QRENDER_PRIM_TRIANGLES, 
									 0, 
									 0, 
									 m_pFrame->m_vSurfaces[i]->m_vVerts.size(), 
									 m_pFrame->m_vSurfaces[i]->m_vTriangles.size() * 3);
		curVBO->UnbindBuffer();
		curIBO->UnbindBuffer();

		g_pRender->ChangeCullMode(QRENDER_CULL_DEFAULT);
	}
	
	g_pRender->SetMatrix(QRENDER_MATRIX_MODEL, PW);
}

void CMD3Mesh::UpdateModel()
{
	m_iCurrentFrame += 0.05f;
	if(m_iCurrentFrame >= m_vFrames.size())
		m_iCurrentFrame = 0;
	
	m_modelCenter = m_vFrames[m_iCurrentFrame]->LocalOrigin;
}

void CMD3Mesh::GetAABB(vec3f& mins, vec3f& maxs)
{
	mins = m_vFrames[m_iCurrentFrame]->MinBounds;
	maxs = m_vFrames[m_iCurrentFrame]->MaxBounds;
}

void CMD3Mesh::GetBoundingSphere(vec3f& center, float& rad)
{
	center = m_modelCenter;
	rad	   = m_vFrames[m_iCurrentFrame]->Radius;
}

void CMD3Mesh::GetModelCenter(vec3f& center)
{
	center = m_modelCenter;
}

bool CMD3Mesh::IsInstance()
{
	return false;
}

bool CMD3Mesh::LoadEffect(const std::string& fxName, const std::string& fxPath)
{
	return true;
}

////////////////////////////////////////////////////////////

CMD3Model::CMD3Model(const unsigned int handle, const std::string& name, const std::string& path)
: CModelObject(handle, name, path)
{
	m_modelCenter.set(0.0f, 0.0f, 0.0f);
	m_worldPos.set(0.0f, 0.0f, 0.0f);
	m_modelScale.set(1.0f, 1.0f, 1.0f);
	QMATH_MATRIX_LOADIDENTITY(m_modelPose);
}

CMD3Model::~CMD3Model()
{
	
}

bool CMD3Model::LoadModel()
{
	m_pXML = new CXML(m_fileName);
	
	unsigned int numModel = m_pXML->GetNumNodes("model");
	
	if(numModel == 0)
		return false;
	
	const CXMLNode* pModel = m_pXML->GetNode("model");
	
	
	
	return true;
}

void CMD3Model::RenderModel()
{
	
}

void CMD3Model::UpdateModel()
{

}

void CMD3Model::GetAABB(vec3f& mins, vec3f& maxs)
{

}

void CMD3Model::GetBoundingSphere(vec3f& center, float& rad)
{

}

void CMD3Model::GetModelCenter(vec3f& center)
{

}

bool CMD3Model::IsInstance()
{
	return true;
}

// void CMD3Model::CreateFinalTransform(mat4& M)
// {
// 	
// }

//CQuadrionEffect* CMD3Model::m_md3Shader = NULL;
//
//CMD3Model::CMD3Model(const unsigned handle, const std::string &name, const std::string &path)
// : CQuadrionModel(handle, name, path)
//{
//	m_eModelType = MODEL_TYPE_UNKNOWN;
//	QMATH_MATRIX_LOADIDENTITY(m_m4PreviousTransform);
//	m_quatRotation.set(0.0f, 0.0f, 0.0f, 0.0f);
//	m_v3Position.set(0.0f, 0.0f, 0.0f);
//	m_v3Scale.set(1.0f, 1.0f, 1.0f);
//	m_fFrame = 0.0f;
//
//	m_pBaseTexture = NULL;
//	m_pLightMapTexture = NULL;
//
//	m_pLink = NULL;
//
//	m_pRenderer = NULL;
//
//	m_pAnimationTimer = NULL;
//
//	m_fAccumulator = 0.0f;
//}
//
//CMD3Model::~CMD3Model()
//{
//	for(unsigned int i = 0; i < m_vSurfaces.size(); ++i)
//	{
//		if(m_vSurfaces[i])
//		{
//			delete m_vSurfaces[i];
//			m_vSurfaces[i] = NULL;
//		}
//	}
//	m_vSurfaces.clear();
//
//	for(unsigned int i = 0; i < m_vFrames.size(); ++i)
//	{
//		if(m_vFrames[i])
//		{
//			delete m_vFrames[i];
//			m_vFrames[i] = NULL;
//		}
//	}
//	m_vFrames.clear();
//
//	for(unsigned int i = 0; i < m_vTags.size(); ++i)
//	{
//		if(m_vTags[i])
//		{
//			delete m_vTags[i];
//			m_vTags[i] = NULL;
//		}
//	}
//	m_vTags.clear();
//
//	m_pAnimationTimer->Stop();
//	delete m_pAnimationTimer;
//}
//
//bool CMD3Model::LoadModel()
//{
//	std::ifstream file((m_sPath+m_sName).c_str(), std::ios::binary);
//	if(!file.is_open())
//		return false;
//	
//	file.seekg(0);
//	
//	SMD3Header *pHeader = new SMD3Header;
//	
//	file.read((char*)pHeader, sizeof(SMD3Header));
//	
//	if(pHeader->Ident != MD3_IDENT)
//	{
//		delete pHeader;
//		pHeader = NULL;
//		return false;
//	}
//
//	file.seekg(pHeader->OffsetFrames);
//	
//	for(unsigned int i = 0; i < pHeader->NumFrames; ++i)
//	{
//		SMD3Frame *pFrame = new SMD3Frame;
//		file.read((char*)pFrame, sizeof(SMD3Frame));
//		m_vFrames.push_back(pFrame);
//	}
//
//	file.seekg(pHeader->OffsetTags);
//
//	for(unsigned int i = 0; i < pHeader->NumTags * pHeader->NumFrames; ++i)
//	{
//		SMD3Tag *pTag = new SMD3Tag;
//		file.read((char*)pTag, sizeof(SMD3Tag));
//		m_vTags.push_back(pTag);
//	}
//
//	m_iNumTags = pHeader->NumTags;
//
//	file.seekg(pHeader->OffsetSurfaces);
//
//	std::vector<SMD3Surface*> MD3Surfaces;
//	std::vector<SMD3Triangle*> MD3Triangles;
//	std::vector<SMD3Shader*> MD3Shaders;
//	std::vector<SMD3TexCoord*> MD3TexCoords;
//	std::vector<SMD3Vertex*> MD3Verts;
//
//	for(unsigned int i = 0; i < pHeader->NumSurfaces; ++i)
//	{
//		SMD3Surface *pMD3Surface = new SMD3Surface;
//
//		int surfaceStart = file.tellg();
//
//		file.read((char*)pMD3Surface, sizeof(SMD3Surface));
//
//		//	Not a valid surface? Cleanup and skip.
//		if(pMD3Surface->Ident != MD3_IDENT)
//		{
//			delete pMD3Surface;
//			pMD3Surface = NULL;
//			continue;
//		}
//
//		MD3Surfaces.push_back(pMD3Surface);
//
//		//	Read in triangles for this surface.
//		file.seekg(surfaceStart + MD3Surfaces[i]->OffsetTriangles);
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumTriangles; ++j)
//		{
//			SMD3Triangle *pTriangle = new SMD3Triangle;	
//			file.read((char*)pTriangle, sizeof(SMD3Triangle));
//			MD3Triangles.push_back(pTriangle);
//		}
//
//		//	Read in shaders for this surface.
//		file.seekg(surfaceStart + MD3Surfaces[i]->OffsetShaders);
//		
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumShaders; ++j)
//		{
//			SMD3Shader *pShader = new SMD3Shader;
//			file.read((char*)pShader, sizeof(SMD3Shader));
//			MD3Shaders.push_back(pShader);
//		}
//
//		//	Read in texture coordinates for this surface.
//		file.seekg(surfaceStart + MD3Surfaces[i]->OffsetST);
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumVerts; ++j)
//		{
//			SMD3TexCoord *pTexCoord = new SMD3TexCoord;
//			file.read((char*)pTexCoord, sizeof(SMD3TexCoord));
//			MD3TexCoords.push_back(pTexCoord);
//		}
//
//		//	Read in the verts
//		file.seekg(surfaceStart + MD3Surfaces[i]->OffsetXYZNormal);
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumVerts * MD3Surfaces[i]->NumFrames; ++j)
//		{
//			SMD3Vertex *pVertex = new SMD3Vertex;
//			file.read((char*)pVertex, sizeof(SMD3Vertex));
//			MD3Verts.push_back(pVertex);
//		}
//
//		//	Jump to the end of the surface after reading.
//		file.seekg(pHeader->OffsetSurfaces + pMD3Surface->OffsetEnd);
//	}
//
//	//	Re-arrange data.
//	for(unsigned int i = 0; i < MD3Surfaces.size(); ++i)
//	{
//		CMD3Surface* pSurface = new CMD3Surface;
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumFrames; ++j)
//		{
//			CMD3SurfaceFrame* pFrame = new CMD3SurfaceFrame;
//			pFrame->SetFrame(m_vFrames[j]);
//
//			//
//			for(unsigned int k = 0; k < MD3Surfaces[i]->NumVerts; ++k)
//			{
//				SMD3VertexF* ptVert = new SMD3VertexF;
//
//				float x = MD3Verts[k]->Position[0] * MD3_XYZ_SCALE * 0.03f;
//				float y = MD3Verts[k]->Position[1] * MD3_XYZ_SCALE * 0.03f;
//				float z = MD3Verts[k]->Position[2] * MD3_XYZ_SCALE * 0.03f;
//				ptVert->Position[0] = x;
//				ptVert->Position[1] = z;
//				ptVert->Position[2] = -y;
//
//				//	Convert normals from Zenith, azimuth to lat long.
//				ptVert->Normal[0] *= (2*QMATH_PI)/255;
//				ptVert->Normal[1] *= (2*QMATH_PI)/255;
//				pFrame->AddVertex(ptVert);
//			}
//
//			MD3Verts.erase(MD3Verts.begin(), MD3Verts.begin()+MD3Surfaces[i]->NumVerts);
//
//			for(unsigned int k = 0; k < pHeader->NumTags; ++k)
//				pFrame->AddTag(m_vTags[j + k]);
//
//			pSurface->AddFrame(pFrame);
//		}
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumTriangles; ++j)
//			pSurface->AddTriangle(MD3Triangles[j]);
//
//		MD3Triangles.erase(MD3Triangles.begin(), MD3Triangles.begin()+MD3Surfaces[i]->NumTriangles);
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumVerts; ++j)
//			pSurface->AddTexCoord(MD3TexCoords[j]);
//
//		MD3TexCoords.erase(MD3TexCoords.begin(), MD3TexCoords.begin()+MD3Surfaces[i]->NumVerts);
//
//		for(unsigned int j = 0; j < MD3Surfaces[i]->NumShaders; ++j)
//			pSurface->AddShader(MD3Shaders[j]);
//
//		MD3Shaders.erase(MD3Shaders.begin(), MD3Shaders.begin()+MD3Surfaces[i]->NumShaders);
//
//		pSurface->SetParentModel(this);
//
//		m_vSurfaces.push_back(pSurface);
//	}
//	
//	MD3Triangles.clear();
//	MD3TexCoords.clear();
//	MD3Shaders.clear();
//	MD3Surfaces.clear();
//
//	file.seekg(pHeader->OffsetEOF);
//
//	//	Sanity clean up. If we have invalid surfaces we may 
//	//  have some 'unclaimed' data that needs to
//	//	be deallocated.
//	for(unsigned int i = 0; i < MD3Surfaces.size(); ++i)
//	{
//		if(MD3Surfaces[i])
//			delete MD3Surfaces[i];
//	}
//	MD3Surfaces.clear();
//
//	for(unsigned int i = 0; i < MD3Triangles.size(); ++i)
//	{
//		if(MD3Triangles[i])
//			delete MD3Triangles[i];
//	}
//	MD3Triangles.clear();
//
//	for(unsigned int i = 0; i < MD3Shaders.size(); ++i)
//	{
//		if(MD3Shaders[i])
//			delete MD3Shaders[i];
//	}
//	MD3Shaders.clear();
//
//	for(unsigned int i = 0; i < MD3TexCoords.size(); ++i)
//	{
//		if(MD3TexCoords[i])
//			delete MD3TexCoords[i];
//	}
//	MD3TexCoords.clear();
//
//	for(unsigned int i = 0; i < MD3Verts.size(); ++i)
//	{
//		if(MD3Verts[i])
//			delete MD3Verts[i];
//	}
//	MD3Verts.clear();
//
//	delete pHeader;
//
//	PreCache();
//
//	m_pAnimationTimer = new CTimer();
//
//	return true;
//}
//
//void CMD3Model::PreCache()
//{
//	SQuadrionVertexDescriptor desc;
//	CQuadrionVertexBuffer* vbo = NULL;
//	CQuadrionIndexBuffer* ibo = NULL;
//
//	desc.pool = QVERTEXBUFFER_MEMORY_STATIC;
//	desc.usage[0] = QVERTEXFORMAT_USAGE_POSITION;
//	desc.size[0] = QVERTEXFORMAT_SIZE_FLOAT3;
//
//	desc.usage[1] = QVERTEXFORMAT_USAGE_NORMAL;
//	desc.size[1] = QVERTEXFORMAT_SIZE_FLOAT3;
//
//	desc.usage[2] = QVERTEXFORMAT_USAGE_TANGENT;
//	desc.size[2] = QVERTEXFORMAT_SIZE_FLOAT3;
//
//	desc.usage[3] = QVERTEXFORMAT_USAGE_TEXCOORD;
//	desc.size[3] = QVERTEXFORMAT_SIZE_FLOAT2;
//
//	desc.usage[4] = QVERTEXFORMAT_USAGE_END;
//
//	for(int i = 0; i < m_vSurfaces.size(); ++i)
//	{
//		std::vector<SMD3TexCoord*>* pvTexCoords = m_vSurfaces[i]->GetTexCoords();
//		std::vector<CMD3SurfaceFrame*>* vFrames = m_vSurfaces[i]->GetFrames();
//		std::vector<SMD3Triangle*>* vTriangles = m_vSurfaces[i]->GetTriangles();
//
//		unsigned short* pIndices = new unsigned short[vTriangles->size() * 3];
//
//		for(unsigned int j = 0; j < vFrames->size(); j++)
//		{
//			CMD3SurfaceFrame* pFrame = vFrames->at(j);
//			pFrame->SetParentSurface(m_vSurfaces[i]);
//
//			std::vector<SMD3VertexF*>* pvVerts = pFrame->GetVertices();
//			SMD3VertexFormat* pVerts = new SMD3VertexFormat[pvVerts->size()];
//			
//			for(unsigned int k = 0; k < pvVerts->size(); ++k)
//			{
//				SMD3VertexF* pVert = pvVerts->at(k);
//
//				pVerts[k].x = pVert->Position[0];
//				pVerts[k].y = pVert->Position[1];
//				pVerts[k].z = pVert->Position[2];
//				pVerts[k].nx = cos(pVert->Normal[0]) * sin(pVert->Normal[1]);
//				pVerts[k].ny = sin(pVert->Normal[0]) * sin(pVert->Normal[1]);
//				pVerts[k].nz = cos(pVert->Normal[1]);
//				pVerts[k].vx = 0;
//				pVerts[k].vy = 0;
//				pVerts[k].vz = 0;
//
//				if(j+1 < vFrames->size())
//				{
//					CMD3SurfaceFrame* pFrameB = vFrames->at(j+1);
//
//					std::vector<SMD3VertexF*>* pvVerts = pFrameB->GetVertices();
//
//					pVerts[k].vx = pvVerts->at(k)->Position[0] - pVerts[k].x;
//					pVerts[k].vy = pvVerts->at(k)->Position[1] - pVerts[k].y;
//					pVerts[k].vz = pvVerts->at(k)->Position[2] - pVerts[k].z;
//				}
//
//				pVerts[k].u = pvTexCoords->at(k)->ST[0];
//				pVerts[k].v = pvTexCoords->at(k)->ST[1];
//			}
//
//			pFrame->SetVBOHandle(m_pRenderer->AddVertexBuffer());
//			vbo = m_pRenderer->GetVertexBuffer(pFrame->GetVBOHandle());
//			vbo->CreateVertexBuffer(pVerts, desc, pvVerts->size(), FALSE);
//
//			delete[] pVerts;
//			pVerts = NULL;
//		}
//		
//		for(unsigned int j = 0; j < vTriangles->size(); j++)
//		{
//			pIndices[j * 3] = vTriangles->at(j)->Indexes[0];
//			pIndices[j * 3 + 1] = vTriangles->at(j)->Indexes[1];
//			pIndices[j * 3 + 2] = vTriangles->at(j)->Indexes[2];
//		}
//
//		m_vSurfaces[i]->SetIBOHandle(m_pRenderer->AddIndexBuffer());
//		ibo = m_pRenderer->GetIndexBuffer(m_vSurfaces[i]->GetIBOHandle());
//		ibo->CreateIndexBuffer(QINDEXBUFFER_MEMORY_STATIC, QINDEXBUFFER_SIZE_USHORT, vTriangles->size() * 3, pIndices);
//		delete[] pIndices;
//		pIndices = NULL;
//	}
//}
//
//void CMD3Model::AddLink(CMD3Model* parentModel, const std::string &pivot, const std::string &anchor)
//{
//	if(GetTag(pivot) && parentModel->GetTag(anchor))
//	{
//		SMD3Link* pLink = new SMD3Link;
//		pLink->Parent = parentModel;
//		pLink->Pivot  = std::string(pivot);
//		pLink->Anchor = std::string(anchor);
//		m_pLink = pLink;
//
//		//m_pParentModelManager->GenerateModelHierarchy();
//
//		//	Anytime we add a link we need to regenerate the model managers MD3Link tree.
//	}
//}
//
//SMD3Link* CMD3Model::GetLink()
//{
//	return m_pLink;
//}
//
//SMD3Tag* CMD3Model::GetTag(const std::string &name)
//{
//	SMD3Tag *pTag = NULL;
//	for(unsigned int i = 0; i < m_iNumTags; ++i)
//	{
//		pTag = m_vTags[(unsigned int)m_fFrame * m_iNumTags + i];
//
//		if(strcmp((const char*)pTag->Name, name.c_str()) == 0)
//			return pTag;
//	}
//
//	return pTag;
//}
//
//void CMD3Model::Initialize(CQuadrionRender* renderer)
//{
//	m_pRenderer = renderer;
//
//	if(!m_md3Shader)
//		m_md3Shader = m_pRenderer->GetEffect("fx/MD3Shader.fx", "./");
//}
//
//CQuadrionRender* CMD3Model::GetRenderer()
//{
//	return m_pRenderer;
//}
//
//void CMD3Model::SetModelManager(CQuadrionModelManager* pModelManager)
//{
//	m_pParentModelManager = pModelManager;
//}
//
//void CMD3Model::PreRender()
//{
//	m_md3Shader->UploadParameters("g_delta", QEFFECT_VARIABLE_FLOAT, 1, &m_fTween);
//
//	mat4 T, R, NT;
//
//	QMATH_QUATERNION_MAKEMATRIX(R, m_quatRotation);
//	m_pRenderer->GetMatrix(QRENDER_MATRIX_MODEL, m_m4PreviousTransform);
//	vec3f centerOffset = vec3f(0.0f, 0.0f, 0.0f);
//
//	QMATH_MATRIX_LOADTRANSLATION(T, vec3f(m_v3Position - centerOffset));
//	QMATH_MATRIX_LOADTRANSLATION(NT, vec3f(centerOffset.x, centerOffset.y, centerOffset.z));
//
//	m_pRenderer->MulMatrix(QRENDER_MATRIX_MODEL, T);
//	m_pRenderer->MulMatrix(QRENDER_MATRIX_MODEL, NT);
//	m_pRenderer->MulMatrix(QRENDER_MATRIX_MODEL, R);
//	QMATH_MATRIX_LOADTRANSLATION(NT, vec3f(-centerOffset.x, -centerOffset.y, -centerOffset.z));
//	m_pRenderer->MulMatrix(QRENDER_MATRIX_MODEL, NT);
//	m_pRenderer->GetMatrix(QRENDER_MATRIX_MODEL, m_m4Last);
//}
//
//void CMD3Model::Render()
//{
//	for(unsigned int j = 0; j < m_vSurfaces.size(); ++j)
//	{
//		m_vSurfaces[j]->RenderFrame(m_fFrame);
//		m_pRenderer->SetMatrix(QRENDER_MATRIX_MODEL, m_m4PreviousTransform);
//	}
//}
//
//void CMD3Model::Update()
//{
//	float dt = 0.0f;
//	if(m_pAnimationTimer->IsRunning)
//	{
//		dt = m_pAnimationTimer->GetElapsedMilliSec();
//		m_pAnimationTimer->Stop();
//		m_pAnimationTimer->Reset();
//	}
//
//	float step = 0.015f;
//
//	m_fFrame += step * dt;
//	if(m_fFrame >= m_vFrames.size())
//		m_fFrame = 0;
//	m_fTween = 0.0f;
//	m_fTween = modf(m_fFrame, &m_fTween);
//	
//	//std::stringstream strBuf;
//	//strBuf << "Tween: " << m_fTween << std::endl;
//	//OutputDebugString(strBuf.str().c_str());
//
//	m_pAnimationTimer->Start();
//
//	//float progAmt = 0.01f;
//	//m_fFrame += progAmt;
//	//
//	//float m_fTween = 0.0f;
//
//	//if(m_fFrame >= m_vFrames.size())
//	//	m_fFrame = 0;
//
//	//m_fTween = modf(m_fFrame, &m_fTween);
//
//
//
//	//m_md3Shader->UploadParameters("g_delta", QEFFECT_VARIABLE_FLOAT, 1, &m_fTween);
//
//	//	TODO: There are some potential issues here, a high dt could make the animation keep resetting to the first frame.
//	//	for looping animation I should probably use some sort of linked list.
//}
//
//void CMD3Model::DebugRenderTags()
//{
//	for(unsigned int i = 0; i < m_iNumTags; ++i)
//	{
//		SMD3Tag *pTag = m_vTags[(unsigned int)m_fFrame * m_iNumTags + i];
//
//		vec3f pos = m_v3Position;
//
//		pos += vec3f(pTag->Origin.x, pTag->Origin.z, -pTag->Origin.y) * 0.03f;
//
//		vec3f posx = pos;
//		vec3f posy = pos;
//		vec3f posz = pos;
//
//		posx -= vec3f(-pTag->Axis[0], pTag->Axis[6], -pTag->Axis[3]) * 0.2f;
//		posy += vec3f(-pTag->Axis[1], pTag->Axis[7], -pTag->Axis[4]) * 0.2f;
//		posz += vec3f(-pTag->Axis[2], pTag->Axis[8], -pTag->Axis[5]) * 0.2f;
//
//		m_pRenderer->RenderLine(pos, posx, QRENDER_MAKE_ARGB(255, 255, 0, 0));
//		m_pRenderer->RenderLine(pos, posy, QRENDER_MAKE_ARGB(255, 0, 255, 0));
//		m_pRenderer->RenderLine(pos, posz, QRENDER_MAKE_ARGB(255, 0, 0, 255));
//	}
//}

//bool CMD3Model::LoadMD3AnimationCfg()
//{
//	//	The animation.cfg should be in the same directory as the model.
//	std::string path = (m_sPath+m_sName);
//
//	size_t ext = path.find_last_of('/');
//	
//	if(ext == std::string::npos)
//		ext = path.find_last_of('\\');
//	
//	if(ext == std::string::npos)
//		return FALSE;
//	
//	path = path.substr(0, ext);
//	
//	path += "/animation.cfg";
//	
//	std::fstream fsFile(path.c_str());
//	
//	if(!fsFile.is_open())
//		return FALSE;
//
//	return true;
//}

//bool CMD3Model::LoadMD3AnimationCfg()
//{
//	std::string tmpLine;
//
//	std::vector<std::string> data;
//
//	while(std::getline(fsFile, tmpLine))
//	{
//		if(tmpLine.size()>0)
//		{
//			size_t comment = tmpLine.find_first_of("//");
//			
//			while(comment != std::string::npos)
//			{
//				tmpLine.erase(tmpLine.begin() + comment, tmpLine.end());
//				
//				if(tmpLine.size() <= 0)
//					break;
//
//				comment = tmpLine.find_first_of("//");
//			}
//
//			while(tmpLine.find_first_of('\t') == 0 || tmpLine.find_first_of(' ') == 0)
//			{
//				tmpLine.erase(tmpLine.begin(), tmpLine.begin()+1);
//				
//				if(tmpLine.size() <= 0)
//					break;
//			}
//
//			if(tmpLine.size() <= 0)
//				continue;
//
//			data.push_back(tmpLine);
//		}
//	}	
//
//	for(int i = 0; i < data.size(); ++i)
//	{
//		std::vector<std::string> tokenized;
//		tokenized = Split("\t", data[i]);
//		
//		if(tokenized.size() != MD3_MAX_ANIMATION_CFG_PARAMS)
//			continue;
//
//		SQMD3Animation *animation = new SQMD3Animation;
//
//		animation->FirstFrame = atoi(tokenized[0].c_str());
//		animation->NumFrames = atoi(tokenized[1].c_str());
//		animation->LoopingFrames = atoi(tokenized[2].c_str());
//		animation->FPS = atoi(tokenized[3].c_str());
//		tokenized[4] = tokenized[4].substr(tokenized[4].find_first_not_of(' '), tokenized[4].find_last_not_of(' ')+1);
//		tokenized[4] = tokenized[4].substr(tokenized[4].find_first_not_of('\t'), tokenized[4].find_last_not_of('\t')+1);
//
//		strcpy(animation->Name,tokenized[4].c_str());
//
//		m_vAnimations.push_back(animation);
//	}
//
//	return TRUE;
//}

//////////////////////////////////////////////////////////////////////////

//CMD3SurfaceFrame::CMD3SurfaceFrame()
//{
//	m_pMD3Frame = NULL;
//	m_pParentSurface = NULL;
//}
//
//CMD3SurfaceFrame::~CMD3SurfaceFrame()
//{
//	m_pMD3Frame = NULL;	//	Frames are deallocated elsewhere.
//
//	for(unsigned int i = 0; i < m_vVertices.size(); ++i)
//	{
//		if(m_vVertices[i])
//		{
//			delete m_vVertices[i];
//			m_vVertices[i] = NULL;
//		}
//	}
//	m_vVertices.clear();
//
//	m_vTags.clear();	//	Tags are deallocated elsewhere.
//}
//
//void CMD3SurfaceFrame::AddTag(SMD3Tag* const tag)
//{
//	if(tag)
//		m_vTags.push_back(tag);
//}
//
//void CMD3SurfaceFrame::AddVertex(SMD3VertexF* const vert)
//{
//	if(vert)
//		m_vVertices.push_back(vert);
//}
//
//void CMD3SurfaceFrame::SetFrame(SMD3Frame* const frame)
//{
//	if(frame)
//		m_pMD3Frame = frame;
//}
//
//const SMD3Frame* const CMD3SurfaceFrame::GetFrameInfo() const
//{
//	return m_pMD3Frame;
//}
//
//std::vector<SMD3VertexF*>* const CMD3SurfaceFrame::GetVertices()
//{
//	return &m_vVertices;
//}
//
//void CMD3SurfaceFrame::SetVBOHandle(const int vbo)
//{
//	m_iVBOHandle = vbo;
//}
//
//const int CMD3SurfaceFrame::GetVBOHandle()
//{
//	return m_iVBOHandle;
//}
//
//void CMD3SurfaceFrame::SetParentSurface(CMD3Surface* psurf)
//{
//	m_pParentSurface = psurf;
//}
//
//void CMD3SurfaceFrame::Render(const unsigned int ibo)
//{
//	CQuadrionRender* pRenderer = m_pParentSurface->GetParentModel()->GetRenderer();
//
//	pRenderer->ChangeCullMode(QRENDER_CULL_NONE);
//
//	CQuadrionVertexBuffer* curVBO = NULL;
//	CQuadrionIndexBuffer* curIBO = NULL;
//
//	curVBO = pRenderer->GetVertexBuffer(m_iVBOHandle);
//	curVBO->BindBuffer();
//
//	curIBO = pRenderer->GetIndexBuffer(ibo);
//	curIBO->BindBuffer();
//	
//	if(m_pParentSurface->GetParentModel()->GetBaseTexture())
//		m_pParentSurface->GetParentModel()->GetBaseTexture()->BindTexture(0);	
//	
//	if(m_pParentSurface->GetParentModel()->GetLightmapTexture())
//		m_pParentSurface->GetParentModel()->GetLightmapTexture()->BindTexture(1);
//
//	pRenderer->RenderIndexedList(QRENDER_PRIM_TRIANGLES, 0, 0, m_vVertices.size(), m_pParentSurface->GetNumTriangles() * 3);
//	curVBO->UnbindBuffer();
//	curIBO->UnbindBuffer();
//
//	pRenderer->ChangeCullMode(QRENDER_CULL_DEFAULT);
//}
//
////////////////////////////////////////////////////////////////////////////
//
//CMD3Surface::CMD3Surface()
//{
//	std::vector<SMD3Triangle*> m_vTriangles = std::vector<SMD3Triangle*>();
//}
//
//CMD3Surface::~CMD3Surface()
//{
//	for(unsigned int i = 0; i < m_vTriangles.size(); ++i)
//	{
//		if(m_vTriangles[i])
//		{
//			delete m_vTriangles[i];
//			m_vTriangles[i] = NULL;
//		}
//	}
//	m_vTriangles.clear();
//
//	for(unsigned int i = 0; i < m_vFrames.size(); ++i)
//	{
//		if(m_vFrames[i])
//		{
//			delete m_vFrames[i];
//			m_vFrames[i] = NULL;
//		}
//	}
//	m_vFrames.clear();
//
//	for(unsigned int i = 0; i < m_vTexCoords.size(); ++i)
//	{
//		if(m_vTexCoords[i])
//		{
//			delete m_vTexCoords[i];
//			m_vTexCoords[i] = NULL;
//		}
//	}
//	m_vTexCoords.clear();
//}
//
//void CMD3Surface::AddTriangle(SMD3Triangle* triangle)
//{
//	if(triangle)
//		m_vTriangles.push_back(triangle);
//}
//
//void CMD3Surface::AddFrame(CMD3SurfaceFrame* frame)
//{
//	if(frame)
//		m_vFrames.push_back(frame);
//}
//
//void CMD3Surface::AddShader(SMD3Shader* shader)
//{
//	if(shader)
//		m_vShaders.push_back(shader);
//}
//
//void CMD3Surface::AddTexCoord(SMD3TexCoord* texcoord)
//{
//	if(texcoord)
//		m_vTexCoords.push_back(texcoord);
//}
//
//std::vector<SMD3Triangle*>* const CMD3Surface::GetTriangles()
//{
//	return &m_vTriangles;
//}
//
//std::vector<CMD3SurfaceFrame*>* const CMD3Surface::GetFrames()
//{
//	return &m_vFrames;
//}
//
//std::vector<SMD3TexCoord*>* const CMD3Surface::GetTexCoords()
//{
//	return &m_vTexCoords;
//}
//
//void CMD3Surface::SetIBOHandle(const int ibo)
//{
//	m_iIBOHandle = ibo;
//}
//
//const int CMD3Surface::GetIBOHandle()
//{
//	return m_iIBOHandle;
//}
//
//void CMD3Surface::RenderFrame(const int frame)
//{
//	if(frame > -1 && frame < m_vFrames.size())
//		m_vFrames[frame]->Render(m_iIBOHandle);
//}
//
//const unsigned int CMD3Surface::GetNumTriangles()
//{
//	return m_vTriangles.size();
//}
//
//void CMD3Surface::SetParentModel(CMD3Model* parentModel)
//{
//	if(parentModel)
//		m_pParentModel = parentModel;
//}
//
//CMD3Model* CMD3Surface::GetParentModel()
//{
//	return m_pParentModel;
//}

//////////////////////////////////////////////////////////////////////////

//////

//CMD3Model::CMD3Model(const std::string& path)
//{
//	Initialize();
//	LoadMD3(path.c_str());
//}
//
//bool CMD3Model::LoadMD3(const char* path)
//{
//	std::ifstream file(path, std::ios::binary);
//	if(!file.is_open())
//		return FALSE;
//
////	strcpy(m_szPath, path);
//
//	file.seekg(0);
//
//	SMD3Header *pHeader = new SMD3Header;
//
//	file.read((char*)pHeader, sizeof(SMD3Header));
//
//	if(pHeader->Ident != MD3_IDENT)
//	{
//		delete pHeader;
//		pHeader = NULL;
//		return FALSE;
//	}
//}
//
//bool CMD3Model::IsValid()
//{
//	return m_bValid;
//}
//
//CMD3Model::~CMD3Model()
//{
//	for(int i = 0; i < m_vMD3Frames.size(); ++i)
//	{
//		if(m_vMD3Frames[i])
//		{
//			delete m_vMD3Frames[i];
//			m_vMD3Frames[i] = NULL;
//		}
//	}
//	m_vMD3Frames.clear();
//
//	for(int i = 0; i < m_vMD3Tags.size(); ++i)
//	{
//		if(m_vMD3Tags[i])
//		{
//			delete m_vMD3Tags[i];
//			m_vMD3Tags[i] = NULL;
//		}
//	}
//	m_vMD3Tags.clear();
//
//	for(int i = 0; i < m_vMD3Surfaces.size(); ++i)
//	{
//		if(m_vMD3Surfaces[i])
//		{
//			delete m_vMD3Surfaces[i];
//			m_vMD3Surfaces[i] = NULL;
//		}
//	}
//	m_vMD3Surfaces.clear();
//
//	for(int i = 0; i < m_vQMD3Frames.size(); ++i)
//	{
//		if(m_vQMD3Frames[i])
//		{
//			delete m_vQMD3Frames[i];
//			m_vQMD3Frames[i] = NULL;
//		}
//	}
//	m_vQMD3Frames.clear();
//
//	for(int i = 0; i < m_vQMD3Surfaces.size(); ++i)
//	{
//		if(m_vQMD3Surfaces[i])
//		{
//			delete m_vQMD3Surfaces[i];
//			m_vQMD3Surfaces[i] = NULL;
//		}
//	}
//	m_vQMD3Surfaces.clear();
//
//	for(int i = 0; i < m_vCMD3LodModels.size(); ++i)
//	{
//		if(m_vCMD3LodModels[i])
//		{
//			delete m_vCMD3LodModels[i];
//			m_vCMD3LodModels[i] = NULL;
//		}
//	}
//	m_vCMD3LodModels.clear();
//};
//
//CQMD3Model::CQMD3Model()
//{
//
//}
//
//CQMD3Model::~CQMD3Model()
//{
//	for(int i = 0; i < m_vCMD3Models.size(); ++i)
//	{
//		if(m_vCMD3Models[i])
//		{
//			delete m_vCMD3Models[i];
//			m_vCMD3Models[i] = NULL;
//		}
//	}
//	m_vCMD3Models.clear();
//
//	for(int i = 0; i < m_vCQMD3TagLinks.size(); ++i)
//	{
//		if(m_vCQMD3TagLinks[i])
//		{
//			delete m_vCQMD3TagLinks[i];
//			m_vCQMD3TagLinks[i] = NULL;
//		}
//	}
//	m_vCQMD3TagLinks.clear();
//}
//
//bool CQMD3Model::LoadModel(const char* path)
//{
//	CMD3Model *pModel = new CMD3Model(path);
//
//	if(!pModel->IsValid())
//	{
//		delete pModel;
//		pModel = NULL;
//	}
//
//	m_vCMD3Models.push_back(pModel);
//	return true;
//}
//
//CQFrame::CQFrame()
//{
//	LocalOrigin.set(0.0f, 0.0f, 0.0f);
//}
//
//CQFrame::~CQFrame()
//{
//	if(m_vVertices.size()>0)
//	{
//		for(int i = 0; i < m_vVertices.size(); ++i)
//		{
//			if(m_vVertices[i])
//			{
//				delete m_vVertices[i];
//				m_vVertices[i] = NULL;
//			}
//		}
//
//		m_vVertices.clear();
//	}
//}
//
//CQSurface::CQSurface()
//{
// 
//}
//
//CQSurface::~CQSurface()
//{
//	if(m_vShaders.size()>0)
//	{
//		for(int i = 0; i < m_vShaders.size(); ++i)
//		{
//			if(m_vShaders[i])
//			{
//				delete m_vShaders[i];
//				m_vShaders[i] = NULL;
//			}
//		}
//
//		m_vShaders.clear();
//	}
//
//	if(m_vTriangles.size()>0)
//	{
//		for(int i = 0; i < m_vTriangles.size(); ++i)
//		{
//			if(m_vTriangles[i])
//			{
//				delete m_vTriangles[i];
//				m_vTriangles[i] = NULL;
//			}
//		}
//
//		m_vTriangles.clear();
//	}
//
//	if(m_vTexcoords.size()>0)
//	{
//		for(int i = 0; i < m_vTexcoords.size(); ++i)
//		{
//			if(m_vTexcoords[i])
//			{
//				delete m_vTexcoords[i];
//				m_vTexcoords[i] = NULL;
//			}
//		}
//
//		m_vTexcoords.clear();
//	}
//
//	if(m_vFrames.size()>0)
//	{
//		for(int i = 0; i < m_vFrames.size(); ++i)
//		{
//			if(m_vFrames[i])
//			{
//				delete m_vFrames[i];
//				m_vFrames[i] = NULL;
//			}
//		}
//
//		m_vFrames.clear();
//	}
//
//	m_pRender->UnloadIndexBuffer(m_iIBOHandle);
//}
//
//CMD3Model::CMD3Model()
//{
//	m_pCurrentAnimation = NULL;
//	m_Position = vec3f(0.0f,0.0f,0.0f);
//	m_uiFrame = 0.0f;
//}
//
//CMD3Model::~CMD3Model()
//{
//	if(m_vMD3Frames.size()>0)
//	{
//		for(int i = 0; i < m_vMD3Frames.size(); ++i)
//		{
//			if(m_vMD3Frames[i])
//			{
//				delete m_vMD3Frames[i];
//				m_vMD3Frames[i] = NULL;
//			}
//		}
//
//		m_vMD3Frames.clear();
//	}
//
//	if(m_vMD3Tags.size()>0)
//	{
//		for(int i = 0; i < m_vMD3Tags.size(); ++i)
//		{
//			if(m_vMD3Tags[i])
//			{
//				delete m_vMD3Tags[i];
//				m_vMD3Tags[i] = NULL;
//			}
//		}
//
//		m_vMD3Tags.clear();
//	}
//
//	if(m_vMD3Surfaces.size()>0)
//	{
//		for(int i = 0; i < m_vMD3Surfaces.size(); ++i)
//		{
//			if(m_vMD3Surfaces[i])
//			{
//				delete m_vMD3Surfaces[i];
//				m_vMD3Surfaces[i] = NULL;
//			}
//		}
//
//		m_vMD3Surfaces.clear();
//	}
//
//	if(m_vAnimations.size()>0)
//	{
//		for(int i = 0; i < m_vAnimations.size(); ++i)
//		{
//			if(m_vAnimations[i])
//			{
//				delete m_vAnimations[i];
//				m_vAnimations[i] = NULL;
//			}
//		}
//
//		m_vAnimations.clear();
//	}
//
//	if(m_vSurfaces.size()>0)
//	{
//		for(int i = 0; i < m_vSurfaces.size(); ++i)
//		{
//			if(m_vSurfaces[i])
//			{
//				delete m_vSurfaces[i];
//				m_vSurfaces[i] = NULL;
//			}
//		}
//
//		m_vSurfaces.clear();
//	}
//}
//
//BOOL CMD3Model::LoadModel(const char* path)
//{
//	std::ifstream file(path, std::ios::binary);
//
//	if(!file.is_open())
//		return FALSE;
//
//	strcpy(m_szPath, path);
//
//	file.seekg(0);
//
//	try
//	{
//		SMD3Header *pHeader = new SMD3Header;
//
//		file.read((char*)pHeader, sizeof(SMD3Header));
//
//		if(pHeader->Ident != MD3_IDENT)
//		{
//			delete pHeader;
//			pHeader = NULL;
//			return FALSE;
//		}
//
//		file.seekg(pHeader->OffsetFrames);
//
//		for(int i = 0; i < pHeader->NumFrames; ++i)
//		{
//			SMD3Frame *pFrame = new SMD3Frame;
//
//			file.read((char*)pFrame, sizeof(SMD3Frame));
//
//			m_vMD3Frames.push_back(pFrame);
//		}
//
//		file.seekg(pHeader->OffsetSurfaces);
//
//		//	TODO: Combine this and the next loop together.
//		for(int i = 0; i < pHeader->NumSurfaces; ++i)
//		{
//			SMD3Surface *pMD3Surface = new SMD3Surface;
//
//			file.read((char*)pMD3Surface, sizeof(SMD3Surface));
//
//			//	Jump to the end of the surface after reading.
//			file.seekg(pHeader->OffsetSurfaces + pMD3Surface->OffsetEnd);
//
//			//	If it's not an MD3 surface then what?
//			if(pMD3Surface->Ident != MD3_IDENT)
//			{
//				delete pMD3Surface;
//				pMD3Surface = NULL;
//				continue;
//			}
//
//			m_vMD3Surfaces.push_back(pMD3Surface);
//		}
//
//		file.seekg(pHeader->OffsetSurfaces);
//
//		for(int i = 0; i < pHeader->NumSurfaces; ++i)
//		{
//			CQSurface *pSurface = new CQSurface;
//
//			int surfaceStart = file.tellg();
//
//			//	Read in triangles for this surface.
//			file.seekg(surfaceStart + m_vMD3Surfaces[i]->OffsetTriangles);
//			for(int j = 0; j < m_vMD3Surfaces[i]->NumTriangles; ++j)
//			{
//				SMD3Triangle* pTriangle = new SMD3Triangle;	
//
//				file.read((char*)pTriangle, sizeof(SMD3Triangle));
//
//				pSurface->m_vTriangles.push_back(pTriangle);
//			}
//
//			//	Read in shaders
//			file.seekg(surfaceStart + m_vMD3Surfaces[i]->OffsetShaders);
//			for(int j = 0; j < m_vMD3Surfaces[i]->NumShaders; ++j)
//			{
//				SMD3Shader* pShader = new SMD3Shader;
//
//				file.read((char*)pShader, sizeof(SMD3Shader));
//
//				pSurface->m_vShaders.push_back(pShader);
//			}
//
//			//	Read in texture coordinates
//			file.seekg(surfaceStart + m_vMD3Surfaces[i]->OffsetSt);
//			while(file.tellg() < (surfaceStart + m_vMD3Surfaces[i]->OffsetVertices))
//			{
//				SMD3TexCoord* pTexCoord = new SMD3TexCoord;
//
//				file.read((char*)pTexCoord, sizeof(SMD3TexCoord));
//
//				pSurface->m_vTexcoords.push_back(pTexCoord);
//			}
//
//			//	Read in vertices
//			file.seekg(surfaceStart + m_vMD3Surfaces[i]->OffsetVertices);
//			for(int j = 0; j < m_vMD3Surfaces[i]->NumFrames; ++j)
//			{
//				CQFrame* m_fFrame = new CQFrame;
//				for(int k = 0; k < m_vMD3Surfaces[i]->NumVerts; ++k)
//				{
//					SMD3Vertex* pVertex = new SMD3Vertex;
//
//					short x, y, z, normal;
//
//					file.read((char*)&x, sizeof(short));
//					file.read((char*)&y, sizeof(short));
//					file.read((char*)&z, sizeof(short));
//
//					file.read((char*)&normal, sizeof(short));
//
//					//	TODO: Replace 0.03f magic number with the map scaling.
//					pVertex->Position[0] = (float)x * MD3_XYZ_SCALE * 0.03f;
//					pVertex->Position[1] = (float)z * MD3_XYZ_SCALE * 0.03f;
//					pVertex->Position[2] = (float)-y * MD3_XYZ_SCALE * 0.03f;
//
//					//	TODO: Decode the vertex normal.
//					pVertex->Normal = normal;
//
//					m_fFrame->m_vVertices.push_back(pVertex);
//				}
//
//				pSurface->m_vFrames.push_back(m_fFrame);
//			}
//
//			file.seekg(surfaceStart + m_vMD3Surfaces[i]->OffsetEnd);
//
//			m_vSurfaces.push_back(pSurface);
//		}
//
//		file.seekg(pHeader->OffsetTags);
//
//		for(int i = 0; i < pHeader->NumTags; ++i)
//		{
//			SMD3Tag *pTag = new SMD3Tag;
//
//			file.read((char*)pTag, sizeof(SMD3Tag));
//
//			m_vMD3Tags.push_back(pTag);
//		}
//
//		file.clear();
//		file.close();
//
//		delete pHeader;
//		pHeader = NULL;
//
//		for(int i = 0; i < m_vMD3Surfaces.size(); ++i)
//		{
//			m_vSurfaces[i]->m_iNumVerts = m_vMD3Surfaces[i]->NumVerts;
//			m_vSurfaces[i]->m_iNumTriangles = m_vMD3Surfaces[i]->NumTriangles;
//		}
//	}
//	
//	catch(std::bad_alloc& ba)
//	{
//		std::string err("Memory exception caught in MD3.CPP of type: ");
//		err.append(ba.what());
//		MessageBoxA(NULL, err.c_str(), "Memory Exception caught", MB_ICONEXCLAMATION | MB_ICONERROR);
//	}
//
//	LoadMD3AnimationCfg();
//
//	PreCache();
//
//	return TRUE;
//}
//
//BOOL CMD3Model::LoadMD3AnimationCfg()
//{
//	//	The animation.cfg should be in the same folder as the model.
//	std::string path = std::string(m_szPath);
//
//	size_t ext = path.find_last_of('/');
//
//	if(ext == std::string::npos)
//		ext = path.find_last_of('\\');
//
//	if(ext == std::string::npos)
//		return FALSE;
//
//	path = path.substr(0, ext);
//
//	path += "/animation.cfg";
//
//	std::fstream fsFile(path.c_str());
//
//	if(!fsFile.is_open())
//		return FALSE;
//
//	std::string tmpLine;
//
//	std::vector<std::string> data;
//
//	while(std::getline(fsFile, tmpLine))
//	{
//		if(tmpLine.size()>0)
//		{
//			size_t comment = tmpLine.find_first_of("//");
//			
//			while(comment != std::string::npos)
//			{
//				tmpLine.erase(tmpLine.begin() + comment, tmpLine.end());
//				
//				if(tmpLine.size() <= 0)
//					break;
//
//				comment = tmpLine.find_first_of("//");
//			}
//
//			while(tmpLine.find_first_of('\t') == 0 || tmpLine.find_first_of(' ') == 0)
//			{
//				tmpLine.erase(tmpLine.begin(), tmpLine.begin()+1);
//				
//				if(tmpLine.size() <= 0)
//					break;
//			}
//
//			if(tmpLine.size() <= 0)
//				continue;
//
//			data.push_back(tmpLine);
//		}
//	}	
//
//	for(int i = 0; i < data.size(); ++i)
//	{
//		std::vector<std::string> tokenized;
//		tokenized = Split("\t", data[i]);
//		
//		if(tokenized.size() != MD3_MAX_ANIMATION_CFG_PARAMS)
//			continue;
//
//		SQMD3Animation *animation = new SQMD3Animation;
//
//		animation->FirstFrame = atoi(tokenized[0].c_str());
//		animation->NumFrames = atoi(tokenized[1].c_str());
//		animation->LoopingFrames = atoi(tokenized[2].c_str());
//		animation->FPS = atoi(tokenized[3].c_str());
//		tokenized[4] = tokenized[4].substr(tokenized[4].find_first_not_of(' '), tokenized[4].find_last_not_of(' ')+1);
//		tokenized[4] = tokenized[4].substr(tokenized[4].find_first_not_of('\t'), tokenized[4].find_last_not_of('\t')+1);
//
//		strcpy(animation->Name,tokenized[4].c_str());
//
//		m_vAnimations.push_back(animation);
//	}
//
//	return TRUE;
//}
//
//void CMD3Model::PreCache()
//{
//	SQuadrionVertexDescriptor desc;
//	CQuadrionVertexBuffer* vbo = NULL;
//	CQuadrionIndexBuffer* ibo = NULL;
//	desc.pool = QVERTEXBUFFER_MEMORY_STATIC;
//	desc.usage[0] = QVERTEXFORMAT_USAGE_POSITION;
//	desc.size[0] = QVERTEXFORMAT_SIZE_FLOAT3;
//	desc.usage[1] = QVERTEXFORMAT_USAGE_TEXCOORD;
//	desc.size[1] = QVERTEXFORMAT_SIZE_FLOAT2;
//	desc.usage[2] = QVERTEXFORMAT_USAGE_END;
//
//	for(int i = 0; i < m_vSurfaces.size(); ++i)
//	{
//		try
//		{
//			CQFrame* pFrame = NULL;
//
//			unsigned short* pIndices = new unsigned short[m_vSurfaces[i]->m_vTriangles.size() * 3];
//
//			for(int j = 0; j < m_vSurfaces[i]->m_vTriangles.size(); j++)
//			{
//				pIndices[j * 3] = m_vSurfaces[i]->m_vTriangles[j]->Indexes[0];
//				pIndices[j * 3 + 1] = m_vSurfaces[i]->m_vTriangles[j]->Indexes[1];
//				pIndices[j * 3 + 2] = m_vSurfaces[i]->m_vTriangles[j]->Indexes[2];
//			}
//
//			for(int j = 0; j < m_vSurfaces[i]->m_vFrames.size(); ++j)
//			{
//				pFrame = m_vSurfaces[i]->m_vFrames[j];
//				SMD3VertexFormat* pVerts = new SMD3VertexFormat[pFrame->m_vVertices.size()];
//
//				for(int k = 0; k < pFrame->m_vVertices.size(); ++k)
//				{
//					SMD3Vertex* pVert = pFrame->m_vVertices[k];
//
//					pVerts[k].x = pVert->Position[0];
//					pVerts[k].y = pVert->Position[1];
//					pVerts[k].z = pVert->Position[2];
//					pVerts[k].u = m_vSurfaces[i]->m_vTexcoords[k]->ST[0];
//					pVerts[k].v = m_vSurfaces[i]->m_vTexcoords[k]->ST[1];
//				}
//
//				pFrame->m_iVBOHandle = m_pRender->AddVertexBuffer();
//				vbo = m_pRender->GetVertexBuffer(pFrame->m_iVBOHandle);
//				vbo->CreateVertexBuffer(pVerts, desc, pFrame->m_vVertices.size(), FALSE);
//
//				delete[] pVerts;
//				pVerts = NULL;
//			}
//
//			m_vSurfaces[i]->m_iIBOHandle = m_pRender->AddIndexBuffer();
//			ibo = m_pRender->GetIndexBuffer(m_vSurfaces[i]->m_iIBOHandle);
//			ibo->CreateIndexBuffer(QINDEXBUFFER_MEMORY_STATIC, QINDEXBUFFER_SIZE_USHORT, m_vSurfaces[i]->m_iNumTriangles * 3, pIndices);
//			delete[] pIndices;
//			pIndices = NULL;
//		}
//		
//		catch(std::bad_alloc& ba)
//		{
//			std::string err("Memory exception caught in MD3.CPP PreCache of type: ");
//			err.append(ba.what());
//			MessageBoxA(NULL, err.c_str(), "Memory Exception", MB_ICONEXCLAMATION | MB_OK);
//		}
//	}
//}
//
//VOID CMD3Model::PreRender()
//{
//	mat4 T, R, NT;
//	quat q;
//
//	m_pRender->getMatrix(QRENDER_MATRIX_MODEL, m_PreviousTransform);`
//	vec3f centerOffset = vec3f(0.0f, 0.0f, 0.0f);
//
//	//	temporary
//	centerOffset.x = 0.25f;
//
//	QMATH_MATRIX_LOADTRANSLATION(T, vec3f(m_Position - centerOffset));
//	QMATH_MATRIX_LOADTRANSLATION(NT, vec3f(centerOffset.x, centerOffset.y, centerOffset.z));
//
//	m_pRender->mulMatrix(QRENDER_MATRIX_MODEL, T);
//	m_pRender->mulMatrix(QRENDER_MATRIX_MODEL, NT);
//	m_pRender->mulMatrix(QRENDER_MATRIX_MODEL, m_Rotation);
//
//	QMATH_MATRIX_LOADTRANSLATION(NT, vec3f(-centerOffset.x, -centerOffset.y, -centerOffset.z));
//
//	m_pRender->mulMatrix(QRENDER_MATRIX_MODEL, NT);
//}
//
//VOID CMD3Model::SetRotation(vec3f &axis, float angle)
//{
//	quat q;
//	QMATH_QUATERNION_ROTATE(q, angle, axis);
//	QMATH_QUATERNION_MAKEMATRIX(m_Rotation, q);
//};
//
//VOID CMD3Model::SetPosition(vec3f pos)
//{
//	m_Position.x=pos.x;
//	m_Position.y=pos.y;
//	m_Position.z=pos.z;
//};
//
//VOID CMD3Model::SetFrame(unsigned int m_fFrame)
//{
//	m_uiFrame = m_fFrame;
//}
//
//VOID CMD3Model::SetAnimation(const char* name)
//{
//	for(int i = 0; i < m_vAnimations.size(); ++i)
//	{
//		if(strcmp(m_vAnimations[i]->Name, name)==0)
//			m_pCurrentAnimation = m_vAnimations[i];
//	}
//}
//
//void CMD3Model::DebugRender()
//{
//	m_pRender->ChangeCullMode(QRENDER_CULL_NONE);
//
//	CQuadrionVertexBuffer* curVBO = NULL;
//	CQuadrionIndexBuffer* curIBO = NULL;
//
//	for(int i = 0; i < m_vSurfaces.size(); ++i)
//	{
//		curVBO = m_pRender->GetVertexBuffer(m_vSurfaces[i]->m_vFrames[m_uiFrame]->m_iVBOHandle);
//		curVBO->BindBuffer();
//
//		curIBO = m_pRender->GetIndexBuffer(m_vSurfaces[i]->m_iIBOHandle);
//		curIBO->BindBuffer();
//
//		m_pRender->RenderIndexedList(QRENDER_PRIM_TRIANGLES, 0, 0, m_vSurfaces[i]->m_vFrames[m_uiFrame]->m_vVertices.size(), m_vSurfaces[i]->m_iNumTriangles*3);
//		curVBO->UnbindBuffer();
//		curIBO->UnbindBuffer();
//	}
//
//	m_pRender->ChangeCullMode(QRENDER_CULL_DEFAULT);
//	m_pRender->SetMatrix(QRENDER_MATRIX_MODEL, m_PreviousTransform);
//}
//
//void CMD3Model::DebugRenderCallback(LPVOID self)
//{
//	CMD3Model* myself = (CMD3Model*)self;
//	myself->DebugRender();
//}
//
//vec3f CMD3Model::GetCenterOffset(const unsigned int surface, const unsigned int m_fFrame)
//{
//	return m_vSurfaces[surface]->m_vFrames[m_fFrame]->LocalOrigin;
//}
