#ifdef QRENDER_EXPORTS
	#define QPHYSICSMESHEXPORT_API		__declspec(dllexport)
#else
	#define QPHYSICSMESHEXPORT_API		__declspec(dllimport)
#endif

#ifndef _qPhysicsMeshLoader_h
#define _qPhysicsMeshLoader_h

#include "Extras/ConvexDecomposition/ConvexBuilder.h"

#include "btBulletDynamicsCommon.h"

#include "LinearMath/btQuickprof.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btGeometryUtil.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"
#include "qscriptable.h"

#include "qmodelobject.h"
#include "q3dsmodel.h"

struct QPHYSICSMESHEXPORT_API qPhysicsMeshVBOIBO
{
	qPhysicsMeshVBOIBO()
	{
		vboRef = QRENDER_INVALID_HANDLE;
		iboRef = QRENDER_INVALID_HANDLE;
	}
	int		vboRef;					// vertex buffer object handle
	int		iboRef;					// index buffer object handle
};

class QPHYSICSMESHEXPORT_API qPhysicsMesh
{
public:
	qPhysicsMesh();
	qPhysicsMesh(CModelObject *mdl);
	qPhysicsMesh(btCollisionShape *_shape);
	virtual ~qPhysicsMesh(){}
	virtual void setModel(CModelObject *mdl);
	virtual void processMesh();
	btCollisionShape *getCollisionShape();

	bool save(const std::string& name, const std::string& path);
	bool load(const std::string& name, const std::string& path);

	void intiVBOIBO();

//protected:
	friend class CModelObject;
	CModelObject *mdl_handle;
	ConvexDecomposition::DecompDesc desc;
	//btCompoundShape *compound;
	btCollisionShape *shape;

	// rendering stuff //
	std::vector<qPhysicsMeshVBOIBO>		meshRenderHandles;

};


class QPHYSICSMESHEXPORT_API qMeshDecomposition3DS : public ConvexDecomposition::ConvexDecompInterface
{
public:
	qMeshDecomposition3DS(c3DSModel *mdl);
	virtual void ConvexDecompResult(ConvexDecomposition::ConvexResult &result);

	ConvexDecomposition::ConvexResult info;
	int mBaseCount;
	int mHullCount;
	btAlignedObjectArray<btConvexHullShape*> m_convexShapes;
	btAlignedObjectArray<btVector3> m_convexCentroids;

	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;
	btAlignedObjectArray<btTriangleMesh*> m_trimeshes;

};


class QPHYSICSMESHEXPORT_API qPhysicsMesh3DS : public qPhysicsMesh
{
public:
	qPhysicsMesh3DS();
	qPhysicsMesh3DS(CModelObject *mdl);
	virtual void setModel(CModelObject *mdl);
	virtual ~qPhysicsMesh3DS(){}
	virtual void processMesh();

protected:
	c3DSModel *mdl_handle;
};


class btDynamicsWorld;
class btBulletWorldImporter;

class QPHYSICSMESHEXPORT_API qPhysicsLoader
{
public:
	qPhysicsLoader(btDynamicsWorld *world);
	virtual ~qPhysicsLoader(){}

	bool loadFile(const std::string& file, const std::string& path);

	btCollisionShape *getCollisionShapeByName(const std::string& name);

private:
	btBulletWorldImporter* fileLoader;
};

#endif