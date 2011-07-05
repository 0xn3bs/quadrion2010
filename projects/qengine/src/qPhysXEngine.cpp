#include <stdlib.h>


#include "qphysicsengine.h"
#include "qPhysXEngine.h"

#include "PxFoundation.h"
#include <PxPhysicsAPI.h>
#include <PxDefaultErrorCallback.h>
#include <PxDefaultAllocator.h> 
#include "extensions/PxExtensionsAPI.h"
#include "PsFile.h"
#include "PsShare.h"
#include "PxTask.h"
#include "PxToolkit.h"
#include "PxDefaultSimulationFilterShader.h"
#include "PxFiltering.h"
#include "PxSceneDesc.h"
#include "PxCudaContextManager.h"

#include "qerrorlog.h"

#include "qRigidBody.h"
#include "qRigidBodyPhysX.h"

#include "qmodelobject.h"

using namespace physx;


qPhysXEngine::qPhysXEngine()
{
}

static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader=PxDefaultSimulationFilterShader;

void qPhysXEngine::init()
{
	//	We want to simulate the physics in a centimeter scale.
	PxTolerancesScale tolerance;
	tolerance.length = 100;
	tolerance.mass = 10;
	tolerance.speed = 1000;
	//

	this->mSDK = PxCreatePhysics(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback, tolerance );

	
	if(this->mSDK == NULL)
		QUIT_ERROR("Failed to initialize PhysX!", "Physics Error!");

	if(!PxInitExtensions(*this->mSDK))
		QUIT_ERROR("PxInitExtensions failed!", "Physics Error!");

	physx::pxtask::CudaContextManagerDesc CudaDesc;
	mCudaContextManager = physx::pxtask::createCudaContextManager(CudaDesc);

	PxSceneDesc sceneDesc(this->mSDK->getTolerancesScale());

	sceneDesc.gravity=PxVec3(0.0f, -983.1f, 0.0f);

    if(!sceneDesc.cpuDispatcher)
	{
		PxDefaultCpuDispatcher* mCpuDispatcher = PxDefaultCpuDispatcherCreate(5);
        
		if(!mCpuDispatcher)
           QUIT_ERROR("PxDefaultCpuDispatcherCreate failed!", "Physics Error!");

        sceneDesc.cpuDispatcher = mCpuDispatcher;
    } 

 	if(!sceneDesc.filterShader)
        sceneDesc.filterShader  = gDefaultFilterShader;

	#ifdef PX_WINDOWS
	if(!sceneDesc.gpuDispatcher && mCudaContextManager)
	{
		sceneDesc.gpuDispatcher = this->mCudaContextManager->getGpuDispatcher();
	}
	#endif

	this->mScene = this->mSDK->createScene(sceneDesc);
	if (!this->mScene)
        QUIT_ERROR("createScene failed!", "Physics Error!");

	this->mScene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0);
	this->mScene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

	PxMaterial* mMaterial = this->mSDK->createMaterial(0.5,0.5,0.5);

	//1) Create ground plane
	PxReal d = 0.0f;	 
	PxTransform pose = PxTransform(PxVec3(0.0f, 0, 0.0f),PxQuat(PxHalfPi, PxVec3(0.0f, 0.0f, 1.0f)));

	PxRigidStatic* plane = this->mSDK->createRigidStatic(pose);
	if (!plane)
			QUIT_ERROR("Create plane failed!", "Physics Error!");

	PxShape* shape = plane->createShape(PxPlaneGeometry(), *mMaterial);
	if (!shape)
		QUIT_ERROR("Create Shape failed!", "Physics Error!");

	this->mScene->addActor(*plane);
}

void qPhysXEngine::step(float dt)
{
	this->mScene->simulate(dt);
	this->mScene->flush();

	while(!this->mScene->fetchResults() )     
	{
		// do something useful        
	}
}

qRigidBody* qPhysXEngine::addRigidBody(float mass, CModelObject *mdl, qPhysicsShape shape)
{
	PxReal density = 1.0f;
	//PxTransform transform(PxVec3(0.0f, 50.0f, 0.0f), PxQuat::createIdentity());
	PxTransform transform(PxVec3(mdl->GetModelPos().x, mdl->GetModelPos().y, mdl->GetModelPos().z), PxQuat::createIdentity());
	PxVec3 dimensions(5,5,5);
	PxBoxGeometry geometry(dimensions);
    
	PxMaterial* mMaterial = this->mSDK->createMaterial(0.3,0.2,0.1);
	//PxMaterial* mMaterial = this->mSDK->createMaterial(0.5,0.5,0.5);

	PxRigidDynamic *actor = PxCreateDynamic(*this->mSDK, transform, geometry, *mMaterial, density);
    //actor->setAngularDamping(0.75);
	//actor->setAngularVelocity(PxVec3(rand()%10,rand()%10,rand()%10)); 

	actor->setAngularVelocity(PxVec3(rand()%5, rand()%5, rand()%5)); 

    actor->setLinearVelocity(PxVec3(rand()%10 - rand()%10,rand()%10 - rand()%10,rand()%10 - rand()%10)); 
	if (!actor)
		QUIT_ERROR("create actor failed!", "Physics Error!");
	
	if(!this->mScene->addActor(*actor))
		QUIT_ERROR("Adding Actor failed!", "Physics Error!");

	//box = actor;
	qRigidBody *body =  (qRigidBody*)(new qRigidBodyPhysX(actor));
	return body;
}