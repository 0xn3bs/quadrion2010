#include "btBulletDynamicsCommon.h"

#include "qcamera.h"

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

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0),0);
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,0,0)));   
	btRigidBody::btRigidBodyConstructionInfo
                groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	this->world->addRigidBody(groundRigidBody);

	this->mShapeDrawer = new qPhysicsShapeDrawer();
}

qphysicsengine::~qphysicsengine()
{
}


btDynamicsWorld *qphysicsengine::getWorld()
{
	return this->world;
}

void qphysicsengine::setGravity(vec3f const& _g)
{
	this->world->setGravity(btVector3(_g.x, _g.y, _g.z));
}

void qphysicsengine::setGravity(float const& x, float const& y, float const& z)
{
	this->world->setGravity(btVector3(x, y, z));
}

btRigidBody *qphysicsengine::addBox(float mass, vec3f pos, vec3f size, CModelObject *_handle)
{
	bool isDynamic = (mass != 0.f);
	//size.z /= 2.0f;
	//size.z -= size.z/2.0f;

	btCollisionShape *shape = new btBoxShape(btVector3(size.x, size.y, size.z));

	btVector3 localInertia(0,0,0);
	if(isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	btTransform tpos;
	tpos.setOrigin(btVector3(pos.x, pos.y, pos.z));
	//btMotionState *myMotionState = new qmotionstate(tpos, _handle);
	btDefaultMotionState* myMotionState =
                new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(pos.x,pos.y,pos.z)));

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);
	cInfo.m_restitution = 0.2f;

	btRigidBody* body = new btRigidBody(cInfo);

	this->world->addRigidBody(body);

	return body;
}

btRigidBody *qphysicsengine::addRigidBody(float mass, vec3f pos, vec3f size, btCollisionShape *shape)
{
	bool isDynamic = (mass != 0.f);
	//size.z /= 2.0f;
	//size.z -= size.z/2.0f;

	//btCollisionShape *shape = new btBoxShape(btVector3(size.x, size.y, size.z));

	btVector3 localInertia(0,0,0);
	if(isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	btTransform tpos;
	tpos.setOrigin(btVector3(pos.x, pos.y, pos.z));
	//btMotionState *myMotionState = new qmotionstate(tpos, _handle);
	btDefaultMotionState* myMotionState =
                new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(pos.x,pos.y,pos.z)));

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);
	cInfo.m_restitution = 0.2f;

	btRigidBody* body = new btRigidBody(cInfo);

	this->world->addRigidBody(body);

	return body;
}

btRigidBody *qphysicsengine::addRigidBody(float mass, CModelObject *model, btCollisionShape *shape)
{
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if(isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	mat4 pose;
	model->GetModelOrientation(pose);
	vec3f pos = model->GetModelPos();
	QMATH_MATRIX_TRANSPOSE(pose);

	btTransform transPose; // GET IT????
	transPose.setFromOpenGLMatrix(pose);
	//btMotionState *myMotionState = new qmotionstate(tpos, _handle);
	//btDefaultMotionState* myMotionState = new btDefaultMotionState(transPose);
	btDefaultMotionState* myMotionState =
	new btDefaultMotionState(btTransform(
	btQuaternion(0,0,0,1),
	btVector3(pos.x,pos.y,pos.z)));
	//btVector3(transPose.getOrigin().getX(), transPose.getOrigin().getY(), transPose.getOrigin().getZ())));

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);
	cInfo.m_restitution = 0.2f;

	btRigidBody* body = new btRigidBody(cInfo);

	this->world->addRigidBody(body);

	return body;
}

void qphysicsengine::step(float const& dt)
{
	this->world->stepSimulation(dt, 10);// might want to change the 5 to something calculated.
}

void qphysicsengine::renderBodies(CCamera *cam)
{
	this->mShapeDrawer->resetStats();
	
	const int numObjects = this->world->getNumCollisionObjects();

	for(int i = 0;i < numObjects;i++)
	{
		btCollisionObject *colObj = this->world->getCollisionObjectArray()[i];

		btVector3 center;
		btScalar radius;
		colObj->getCollisionShape()->getBoundingSphere(center, radius);
		center += colObj->getWorldTransform().getOrigin();

		if( cam->IsSphereInFrustum(vec3f(center.x(), center.y(), center.z()), radius))
		{
			btRigidBody *body = btRigidBody::upcast(colObj);

			if(body && body->getMotionState())
			{	
				// So the vehicle shape hull does not get drawn
				//if( body->getCollisionShape()->getShapeType() <= CYLINDER_SHAPE_PROXYTYPE )
				//{
				//shapeMaterial.SetDiffuseColor( Color::Red );
				//mDevice->SetMaterial( &shapeMaterial );

				btTransform trans = body->getWorldTransform();
				mShapeDrawer->renderShape( body->getCollisionShape(), trans );						
				//}
			}else
			{
				//shapeMaterial.SetDiffuseColor( Color::Blue );
				//mDevice->SetMaterial( &shapeMaterial );

				mShapeDrawer->renderShape( colObj->getCollisionShape(), colObj->getWorldTransform() );						
			}
			//objectDrawnCount++;
		}
	}
}

void qphysicsengine::getWorldAABB(btRigidBody *body, btVector3 &min, btVector3 &max)
{
	body->getAabb(min, max);
}

void qphysicsengine::getLocalAABB(btRigidBody *body, btVector3 &min, btVector3 &max)
{
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	body->getCollisionShape()->getAabb(trans, min, max);
}

void qphysicsengine::updateCenterOfMassOffest(btRigidBody *body, CModelObject *mdl)
{
}

void qphysicsengine::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
}