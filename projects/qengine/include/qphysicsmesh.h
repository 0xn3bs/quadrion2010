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








#include <Windows.h>

#include <ostream>

#include <sstream>

#include <string>


class QPHYSICSMESHEXPORT_API qPhysicsMesh
{
public:
	qPhysicsMesh();
	qPhysicsMesh(CModelObject *mdl);
	virtual ~qPhysicsMesh(){}
	virtual void setModel(CModelObject *mdl);
	virtual void processMesh();
	btCompoundShape *getCompoundShape();

//protected:
	friend class CModelObject;
	CModelObject *mdl_handle;
	ConvexDecomposition::DecompDesc desc;
	btCompoundShape *compound;
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


class QPHYSICSMESHEXPORT_API qPhysicsMeshLoader
{
public:
	qPhysicsMeshLoader();
	virtual ~qPhysicsMeshLoader(){}


private:
};

#endif