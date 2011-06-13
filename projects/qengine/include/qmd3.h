////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// qmd3.h
// 
// Written by Jonathan Bastnagel for Quadrion Engine 1/2009
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __QMD3_H_
#define __QMD3_H_

#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>

#include "qmath.h"
#include "qrender.h"
#include "qresource.h"
#include "qmodelobject.h"
#include "qxml.h"

#include "qmodel.h"	//	TODO: DEPRECATED, REMOVE ASAP

const unsigned int	MD3_MAX_ANIMATION_CFG_NAME = 64;
const unsigned int	MD3_MAX_ANIMATION_CFG_PARAMS = 5;
const unsigned int	MD3_MAX_QPATH = 64;
const unsigned int	MD3_MAX_FRAMES = 1024;
const unsigned int	MD3_MAX_TAGS = 16;
const unsigned int	MD3_MAX_SURFACES = 32;
const unsigned int	MD3_MAX_SHADERS = 256;
const unsigned int	MD3_MAX_VERTS = 4096;
const unsigned int	MD3_MAX_TRIANGLES = 8192;
const float			MD3_XYZ_SCALE = 0.015625f;
const unsigned int	MD3_IDENT = 860898377;
const char			MD3_SIDENT[5] = "IDP3";

#ifdef QRENDER_EXPORTS
#define QMD3MODELEXPORT_API		__declspec(dllexport)
#else
#define QMD3MODELEXPORT_API		__declspec(dllimport)
#endif

struct SMD3VertexFormat
{
	float x, y, z;						// world space position
	float nx, ny, nz;					// normal coords (already normalized)
	float vx, vy, vz;					// x, y and z vector to the next vertex, used for interpolation.
	float u, v;							// texture coords
};

struct SMD3Header
{
	int  Ident;
	int  Version;
	unsigned char Name[MD3_MAX_QPATH];
	int  Flags;		//	Not sure what this is for.
	int  NumFrames;
	int  NumTags;
	int  NumSurfaces;
	int  NumSkins;	//	Not really used.
	int  OffsetFrames;
	int  OffsetTags;
	int  OffsetSurfaces;
	int  OffsetEOF;
};

struct SMD3Frame
{
	vec3f MinBounds;
	vec3f MaxBounds;
	vec3f LocalOrigin;
	float Radius;
	unsigned char Name[16];
};

struct SMD3Tag
{
	unsigned char Name[64];
	vec3f Origin;
	mat3 Axis;
};

struct SMD3Surface
{
	int Ident;
	unsigned char Name[64];
	int Flags;
	int NumFrames;
	int NumShaders;
	int NumVerts;
	int NumTriangles;
	int OffsetTriangles;
	int OffsetShaders;
	int OffsetST;
	int OffsetXYZNormal;
	int OffsetEnd;
};

struct SMD3Shader
{
	unsigned char Name[64];
	int ShaderIndex;
};

struct SMD3Triangle
{
	int Indexes[3];
};

struct SMD3TexCoord
{
	float ST[2];	//	Same as UV?
};

struct SMD3Vertex
{
	short Position[3];
	unsigned char Normal[2];
};

class QMD3MODELEXPORT_API CMD3Surface
{
friend class CMD3Mesh;
public:
	CMD3Surface();
	CMD3Surface(const CMD3Surface& surface);
	~CMD3Surface();

	std::vector<SMD3Triangle*>	m_vTriangles;	//	Shared, delete if LAST reference
	std::vector<SMD3TexCoord*>	m_vTexCoords;	//	Shared, delete if LAST reference
	
	std::vector<SMD3Vertex*>	m_vVerts;
	
private:
	int	m_iVboHandle;
	int	m_iIboHandle;
	
	unsigned int m_iRefId;
};

class QMD3MODELEXPORT_API CMD3Frame : public SMD3Frame
{
friend class CMD3Mesh;
public:
	CMD3Frame();
	~CMD3Frame();
	
	void AddTag(SMD3Tag* pTag);
	void AddSurface(CMD3Surface* pSurface);
	
	CMD3Surface* GetSurface(unsigned int i);
	
private:
	std::vector<SMD3Tag*>		m_vTags;
	std::vector<CMD3Surface*>	m_vSurfaces;
};

class QMD3MODELEXPORT_API CMD3Mesh : public CModelObject
{
public:
	CMD3Mesh(const unsigned int handle, const std::string& name, const std::string& path = "./");
	~CMD3Mesh();

	bool LoadModel();
	void RenderModel();
	void UpdateModel();
	void GetAABB(vec3f& mins, vec3f& maxs);
	void GetBoundingSphere(vec3f& center, float& rad);
	void GetModelCenter(vec3f& center);
	bool IsInstance();

	bool LoadEffect(const std::string& fxName, const std::string& fxPath = "./");

	//void CreateFinalTransform(mat4& M);

private:
	float m_iCurrentFrame;
	
	std::vector<CMD3Frame*> m_vFrames;
	
	int m_iTexRef;
	
	std::string m_sPath;
};

class QMD3MODELEXPORT_API CMD3Model : public CModelObject
{
public:
	CMD3Model(const unsigned int handle, const std::string& name, const std::string& path = "./");
	~CMD3Model();
	
	bool LoadModel();
	void RenderModel();
	void UpdateModel();
	void GetAABB(vec3f& mins, vec3f& maxs);
	void GetBoundingSphere(vec3f& center, float& rad);
	void GetModelCenter(vec3f& center);
	bool IsInstance();
	
	bool LoadEffect(const std::string& fxName, const std::string& fxPath = "./");
private:
	CXML* m_pXML;
};

//class CMD3Surface;

//class CMD3SurfaceFrame
//{
//public:
//	CMD3SurfaceFrame();
//	~CMD3SurfaceFrame();
//
//	void AddVertex(SMD3VertexF* const vert);
//	void AddTag(SMD3Tag* const tag);
//	void SetFrame(SMD3Frame* const frame);
//
//	const SMD3Frame* const GetFrameInfo() const;
//	std::vector<SMD3VertexF*>* const GetVertices();
//
//	void SetVBOHandle(const int vbo);
//	const int GetVBOHandle();
//
//	void SetParentSurface(CMD3Surface* psurf);
//
//	void Render(const unsigned int ibo);
//private:
//	SMD3Frame* m_pMD3Frame;	//	Use this to obtain raw min-max bounds, local origin, radius, etc of the frames.
//	std::vector<SMD3VertexF*> m_vVertices;
//	std::vector<SMD3Tag*>	 m_vTags;
//
//	CMD3Surface* m_pParentSurface;
//
//	int m_iVBOHandle;
//};

//class CMD3Model;	//	Forward declaration;
//
//class QMD3MODELEXPORT_API CMD3Surface
//{
//public:
//	CMD3Surface();
//	~CMD3Surface();
//
//	void AddTriangle(SMD3Triangle* triangle);
//	void AddFrame(CMD3SurfaceFrame* frame);
//	void AddShader(SMD3Shader* shader);
//	void AddTexCoord(SMD3TexCoord* texcoord);
//
//	std::vector<SMD3Triangle*>* const GetTriangles();
//	std::vector<CMD3SurfaceFrame*>* const GetFrames();
//	std::vector<SMD3TexCoord*>* const GetTexCoords();
//
//	void SetIBOHandle(const int ibo);
//	const int GetIBOHandle();
//
//	void RenderFrame(const int frame);
//
//	void SetParentModel(CMD3Model* parentModel);
//	CMD3Model* GetParentModel();
//
//	const unsigned int GetNumTriangles();
//private:
//	CMD3Model*	m_pParentModel;
//	std::vector<SMD3Triangle*> m_vTriangles;
//	std::vector<CMD3SurfaceFrame*> m_vFrames;
//	std::vector<SMD3Shader*>   m_vShaders;
//	std::vector<SMD3TexCoord*> m_vTexCoords;
//
//	int m_iIBOHandle;
//};

// 
// struct QMD3MODELEXPORT_API SMD3Link
// {
// 	CMD3Model* Parent;
// 	std::string Pivot, Anchor;
// };

//struct QMD3MODELEXPORT_API CMD3TagPos
//{
//	vec3f pos;
//};

// class QMD3MODELEXPORT_API CMD3Model : public CModelObject
// {
// public:
// 	CMD3Model(const unsigned handle, const std::string &name, const std::string &path = "./");
// 	~CMD3Model();
// 
//  	bool LoadModel();
//  	void PreCache();
// 	
// 	void AddLink(CMD3Model* parentModel, const std::string &pivot, const std::string &anchor);
// 	SMD3Link* GetLink();
// 
// 	SMD3Tag* GetTag(const std::string &name);	//	Gets the tag at the current frame.
// 
// 	void Initialize(CQuadrionRender* renderer);
// 	CQuadrionRender* GetRenderer();
// 
// 	void SetModelManager(CQuadrionModelManager* pModelManager);
// 
// 	void PreRender();
// 	void Render();
// 	void Update();
// 	void DebugRenderTags();
// 
// 	static void RenderCallback(LPVOID self);
// private:
// 	int	m_iVertexFormatHandle;
// 	float m_fFrame;
// 	unsigned int  m_iNumTags;
// 	std::vector<CMD3Surface*> m_vSurfaces;
// 	std::vector<SMD3Frame*> m_vFrames;
// 	std::vector<SMD3Tag*> m_vTags;
// 	std::vector<CMD3TagPos*> m_vTagPositions;
// 	SMD3Link* m_pLink;
// 
// 	CQuadrionRender* m_pRenderer;
// 
// 	mat4 m_m4Last;
// 
// 	CTimer* m_pAnimationTimer;
// 
// 	static CQuadrionEffect* m_md3Shader;
// 
// 	float m_fAccumulator;
// 	float m_fTween;
// 
// 	CQuadrionModelManager* m_pParentModelManager;
// };

////
//
//struct SQMD3Animation
//{
//	unsigned short FirstFrame;
//	unsigned short NumFrames;
//	unsigned short LoopingFrames;
//	unsigned short FPS;
//	char Name[MD3_MAX_ANIMATION_CFG_NAME];
//};
//
//struct SMD3Header
//{
//	int  Ident;
//	int  Version;
//	char Name[MD3_MAX_QPATH];
//	int  Flags;		//	Not sure what this is for.
//	int  NumFrames;
//	int  NumTags;
//	int  NumSurfaces;
//	int  NumSkins;	//	Not really used.
//	int  OffsetFrames;
//	int  OffsetTags;
//	int  OffsetSurfaces;
//	int  OffsetEOF;
//};
//
//struct SMD3Frame
//{
//	vec3f MinBounds;
//	vec3f MaxBounds;
//	vec3f LocalOrigin;
//	float Radius;
//	char  Name[16];
//};
//
//struct SMD3Tag
//{
//	char  Name[MD3_MAX_QPATH];
//	vec3f Origin;
//	vec3f Axis[3];
//};
//
//struct SMD3Surface
//{
//	int  Ident;
//	char Name[MD3_MAX_QPATH];
//	int  Flags;
//	int  NumFrames;
//	int  NumShaders;
//	int  NumVerts;
//	int  NumTriangles;
//	int  OffsetTriangles;
//	int  OffsetShaders;
//	int  OffsetSt;
//	int  OffsetVertices;
//	int  OffsetEnd;
//};
//
//struct SMD3Shader
//{
//	char Name[MD3_MAX_QPATH];
//	int  ShaderIndex;
//};
//
//struct SMD3Triangle
//{
//	int Indexes[3];
//};
//
//struct SMD3TexCoord
//{
//	float ST[2];	//	Same as UV?
//};
//
//struct SMD3Vertex
//{
//	float Position[3];
//	short Normal;
//};
//
//struct SQMD3TagLink
//{
//	std::map<SMD3Surface*, SMD3Tag*> ParentTag;
//	std::map<SMD3Surface*, SMD3Tag*> ChildTag;	//	The child's orientation and movement are affected by the parent's tags.
//};
//
//class CQMD3Frame
//{
//private:
//	SMD3Frame* m_pMD3Frame;	//	Use this to obtain raw min-max bounds, local origin, radius, etc of the frames.
//	std::vector<SMD3Vertex*> m_vVertices;
//	std::vector<SMD3Tag*>	 m_vTags;
//};
//
//class CQMD3Surface
//{
//private:
//	char m_szName[MD3_MAX_QPATH];
//
//	std::vector<CQMD3Frame*> m_vQMD3Frames;
//	std::vector<SMD3Shader*> m_vMD3Shaders;
//
//	int m_iVBOHandle;
//};
//
//
//class CMD3Model
//{
//friend class CQMD3Model;
//
//private:
//	char m_szName[MD3_MAX_QPATH];	//	Usually the same as the path.
//
//	vec3f		m_Position;
//	vec3f		m_Scale;
//	mat4		m_Rotation;
//	mat4		m_PreviousTransform;
//
//	std::vector<SMD3Frame*>		m_vMD3Frames;
//	std::vector<SMD3Tag*>		m_vMD3Tags;
//	std::vector<SMD3Surface*>	m_vMD3Surfaces;
//
//	std::vector<CQMD3Frame*>	m_vQMD3Frames;
//	std::vector<CQMD3Surface*>	m_vQMD3Surfaces;
//	
//	//	If this is the highest LOD model then this list will contain lower LOD's, otherwise it will be empty.
//	//	The lower LOD models can be used for physics.
//	std::vector<CMD3Model*>			m_vCMD3LodModels;
//
//	int	m_iVertexFormatHandle;
//	bool m_bValid;
//
//	void Initialize();
//
//public:
//	CMD3Model();
//	CMD3Model(const std::string& path);
//	~CMD3Model();
//
//	void PreCache();
//	void DebugRender();
//	static void DebugRenderCallback(LPVOID self);
//
//	bool LoadMD3(const char* path);
//	bool IsValid();
//};
//
//class CQMD3Model
//{
//private:
//	//	Different MD3's may be pieced together with tags, if so they must be in this list.
//	std::vector<CMD3Model*>			m_vCMD3Models;
//	//CQuadrionResourceManager<CQuadrionEffect>*	m_ModelResources;
//
//	//	We want the ability of dynamically adding and destroying tag links on models.
//	std::vector<SQMD3TagLink*>	m_vCQMD3TagLinks;
//public:
//	CQMD3Model();
//	~CQMD3Model();
//
//	//	Load a model and add it to the list.
//	bool LoadModel(const char* path);
//};
//
//class CQFrame
//{
//public:
//	CQFrame();
//	~CQFrame();
//	 
//	std::vector<SMD3Vertex*> m_vVertices;
//	
//	vec3f LocalOrigin;
//
//	int m_iVBOHandle;
//};
//
//class CQSurface
//{
//public:
//	CQSurface();
//	~CQSurface();
//
//	std::string m_sName;
//	std::vector<SMD3Shader*> m_vShaders;
//	std::vector<SMD3Triangle*> m_vTriangles;
//	std::vector<SMD3TexCoord*> m_vTexcoords;
//
//	int m_iIBOHandle;
//	int m_iNumVerts;
//	int m_iNumTriangles;
//
//	std::vector<CQFrame*> m_vFrames;
//};
//
//class CMD3Model
//{
//private:
//	std::vector<SMD3Frame*> m_vMD3Frames;
//	std::vector<SMD3Tag*> m_vMD3Tags;
//	std::vector<SMD3Surface*> m_vMD3Surfaces;
//	std::vector<SQAnimation*> m_vAnimations;
//
//	std::vector<CQSurface*> m_vSurfaces;
//
//	mat4		m_ModelMatrix;
//	vec3f		m_Position;
//	vec3f		m_Scale;
//	mat4		m_Rotation;
//	mat4		m_PreviousTransform;
//
//	SQAnimation* m_pCurrentAnimation;
//
//	unsigned int m_uiFrame;
//	int m_VertexFormatHandle;
//
//	char m_szPath[MD3_MAX_QPATH];
//
//public:
//	CMD3Model();
//	~CMD3Model();
//
//	
//	BOOL LoadMD3AnimationCfg();
//	BOOL LoadModel(const char* path);
//	void PreCache();
//
//	void DebugRender();
//
//	static void DebugRenderCallback(LPVOID self);
//
//	vec3f GetCenterOffset(const unsigned int surface, const unsigned int m_fFrame);
//
//	VOID PreRender();
//	VOID SetRotation(vec3f &axis, float angle);
//	VOID SetPosition(vec3f pos);
//
//	VOID SetFrame(unsigned int m_fFrame);
//	VOID SetAnimation(const char* name);	//	Set the active animation.
//	VOID PlayAnimation();	//	Play the active animation.
//	VOID PauseAnimation();	//	Pause the active animation.
//	VOID StopAnimation();	//	Goes to the active animation's starting m_fFrame and pauses.
//};

#endif /*__QMD3_H_*/
