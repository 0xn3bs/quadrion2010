#include "btBulletDynamicsCommon.h"
#include "qscriptengine.h"
#include "qmotionstate.h"
#include "qmodelobject.h"

#include "qphysics.h"

qphysicsengine::qphysicsengine()
{
	this->broadphase = new btDbvtBroadphase();
	this->collisionConfiguration = new btDefaultCollisionConfiguration();
	this->dispatcher = new btCollisionDispatcher(collisionConfiguration);
	this->solver = new btSequentialImpulseConstraintSolver;

	this->world = new btDiscreteDynamicsWorld(this->dispatcher,
													this->broadphase,
													this->solver,
													this->collisionConfiguration);

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),1);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
        btRigidBody::btRigidBodyConstructionInfo
                groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
        btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
        this->world->addRigidBody(groundRigidBody);
}

qphysicsengine::~qphysicsengine()
{
}

void qphysicsengine::setGravity(vec3f const& _g)
{
	this->world->setGravity(btVector3(_g.x, _g.y, _g.z));
}

void qphysicsengine::setGravity(float const& x, float const& y, float const& z)
{
	this->world->setGravity(btVector3(x, y, z));
}

btRigidBody *qphysicsengine::addCube(float mass, vec3f pos, CModelObject *_handle)
{
	bool isDynamic = true;//= (mass != 0.f);
	
	btCollisionShape *shape = new btBoxShape(btVector3(1,1,1));

	btScalar m = 1;
	btVector3 localInertia(0,0,0);
	if(isDynamic)
		shape->calculateLocalInertia(m, localInertia);

	btTransform tpos;
	tpos.setOrigin(btVector3(pos.x, pos.y, pos.z));
	//btMotionState *myMotionState = new qmotionstate(tpos, _handle);
	btDefaultMotionState* myMotionState =
                new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0)));
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);

	this->world->addRigidBody(body);

	return body;
}

void qphysicsengine::step(float const& dt)
{
	this->world->stepSimulation(dt, 5);// might want to change the 5 to something calculated.
}

void qphysicsengine::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
}