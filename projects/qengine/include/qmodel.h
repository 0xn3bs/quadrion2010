#ifndef __QMODEL_H_
#define __QMODEL_H_

#include "qrender.h"
#include "qmath.h"
#include "qeffect.h"
#include "qalgorithm.h"
#include "qxml.h"
#include "qtimer.h"

#include <string>
#include <vector>

#ifdef QRENDER_EXPORTS
#define QMODELEXPORT_API		__declspec(dllexport)
#else
#define QMODELEXPORT_API		__declspec(dllimport)
#endif

const unsigned int	MODEL_XML_ASSEMBLER_MAJOR	= 0;
const unsigned int	MODEL_XML_ASSEMBLER_MINOR	= 1;
const unsigned int	MODEL_XML_ASSEMBLER_BUGFIX	= 0;

enum QMODELEXPORT_API EModelType
{
	MODEL_TYPE_UNKNOWN	= 0x000000000,
	MODEL_TYPE_MD3		= 0x000000001,
	MODEL_TYPE_3DS		= 0x000000002
};

class QMODELEXPORT_API CQuadrionModel
{
public:
	CQuadrionModel(const unsigned int handle, const std::string& name, const std::string& path = "./");
	~CQuadrionModel();

	virtual void PreRender(){};
	virtual void Render(){};
	virtual void Update(){};
	virtual void SetPosition(vec3f &pos);
	virtual void SetRotation(vec3f &axis, float angle);
	virtual void DebugRenderTags() = 0;

	virtual const quat GetRotation();
	virtual vec3f GetPosition();

	virtual bool LoadModel() = 0;

	void SetRender(void* render);

	void SetBaseTexture(CQuadrionTextureObject* pTex);
	void SetLightmapTexture(CQuadrionTextureObject* pTex);

	CQuadrionTextureObject* GetBaseTexture();
	CQuadrionTextureObject* GetLightmapTexture();

	const EModelType GetModelType();
	void SetModelType(const EModelType type);

	static void RenderCallback(LPVOID self);
private:
protected:
	EModelType  m_eModelType;

	mat4		m_m4PreviousTransform;
	quat		m_quatRotation;
	vec3f		m_v3Position;
	vec3f		m_v3Scale;

	std::string m_sName, m_sPath;

	CQuadrionRender* m_pRenderer;

	CQuadrionTextureObject *m_pBaseTexture, *m_pLightMapTexture;
};

//	Forward declarations
class CMD3Mesh;
struct SMD3Link;

class QMODELEXPORT_API CQuadrionModelManager
{
public:
	CQuadrionModelManager(CQuadrionRender* render);
	~CQuadrionModelManager();

	void PreRender(CQuadrionEffect* effect);
	void Render();
	void Update();
	void DebugRenderTags();
	
	int AddModel(const std::string& name, const std::string& path);
	void SetRender(CQuadrionRender* render);

	CQuadrionModel* GetModel(unsigned int handle);
	CQuadrionModel* GetModel(const std::string& name, const std::string& path);
private:
	CQuadrionRender* m_pRenderer;

	std::vector<CMD3Mesh*> m_vModelPool;
	std::vector<unsigned int> m_vHandles;

	CTree<SMD3Link*> m_vMD3LinkTree;

	unsigned int m_iHandleCount;

	CTimer* m_pDeltaTimer;

	float m_fLastTime;
	float m_fDeltaTime;
};

class QMODELEXPORT_API CQuadrionModelAssembler
{
public:
	CQuadrionModelAssembler();
	CQuadrionModelAssembler(std::string path);
	~CQuadrionModelAssembler();

	const int Assemble(std::string path);

private:
	std::vector<CQuadrionModel*> m_vModels;
};

class QMODELEXPORT_API CQuadrionModelObject
{
public:
	CQuadrionModelObject();
	~CQuadrionModelObject();
private:
	CQuadrionModelAssembler* m_pModelAssembler;
};

#endif /*__QMODEL_H_*/
