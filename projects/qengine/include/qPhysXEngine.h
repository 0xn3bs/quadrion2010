#ifndef _qPhysXEngine_h
#define _qPhysXEngine_h

#ifdef QRENDER_EXPORTS
	#define QPHYSXENGINEEXPORT_API		__declspec(dllexport)
#else
	#define QPHYSXENGINEEXPORT_API		__declspec(dllimport)
#endif


#include <vector>
#include "qphysicsengine.h"


/*#include "PxPhysicsAPI.h"
#include "PxDefaultErrorCallback.h"
#include "PxDefaultAllocator.h"*/

class PxDefaultCpuDispatcher;
class PxDefaultErrorCallback;
class PxDefaultAllocator;
class PxPhysics;
class PxCooking;
class PxScene;
class PxGeometry;
class PxMaterial;
class PxRigidActor;
namespace physx
{
	namespace pxtask
	{
class CudaContextManager;

	};
};

class QPHYSXENGINEEXPORT_API qPhysXEngine : public qPhysicsEngine										
{
public:
	qPhysXEngine();
	virtual ~qPhysXEngine(){}

	virtual void		init();
	virtual void		step(float dt);
	virtual qRigidBody*	addRigidBody(float mass, CModelObject *mdl, qPhysicsShape shape);

private:
	PxPhysics* mSDK;
	PxScene* mScene;
	physx::pxtask::CudaContextManager *mCudaContextManager;
};

#endif