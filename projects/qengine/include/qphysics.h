/*
 * Physics engine
 */

#ifdef QRENDER_EXPORTS
	#define QPHYSICSEXPORT_API		__declspec(dllexport)
#else
	#define QPHYSICSEXPORT_API		__declspec(dllimport)
#endif

#ifndef _qphysics_h
#define _qphysics_h

//#include "btBulletDynamicsCommon.h"
#include "qmath.h"
#include "qmodelobject.h"
#include "qobject.h"
#include "qphysicsshapedrawer.h"
#include "qphysicsengine.h"

class qscriptengine;

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btDynamicsWorld;
class btRigidBody;
class btVector3;
class CModelObject;
class btCompoundShape;
class btCollisionShape;

class QPHYSICSEXPORT_API qphysicsengine// : public qPhysicsEngine
{
public:
	qphysicsengine();
	virtual ~qphysicsengine(){}

	virtual void	init();
	virtual void	step(float dt);
	virtual void*	addRigidBody(float mass, CModelObject *mdl, qPhysicsShape shape);

	btDynamicsWorld *getWorld();

	void setGravity(vec3f const& _g);
	void setGravity(float const& x, float const& y, float const& z);

	btRigidBody *addBox(float mass, vec3f pos, vec3f size, CModelObject *_handle);
	btRigidBody *addRigidBody(float mass, vec3f pos, vec3f size, btCollisionShape *shape);
	btRigidBody *addRigidBody(float mass, CModelObject *model, btCollisionShape *shape);

	void renderBodies(CCamera *cam);

	void getWorldAABB(btRigidBody *body, btVector3 &min, btVector3 &max);
	void getLocalAABB(btRigidBody *body, btVector3 &min, btVector3 &max);
	void updateCenterOfMassOffest(btRigidBody *body, CModelObject *mdl);

	virtual void REGISTER_SCRIPTABLES(qscriptengine *engine);
private:

	btAlignedObjectArray<btCollisionShape*>	m_collisionShapes;

	class	btThreadSupportInterface*		m_threadSupportCollision;
	class	btThreadSupportInterface*		m_threadSupportSolver;

	btCollisionAlgorithmCreateFunc*	m_boxBoxCF;

	btBroadphaseInterface				*broadphase;
	btDefaultCollisionConfiguration		*collisionConfiguration;
	btCollisionDispatcher				*dispatcher;
	btSequentialImpulseConstraintSolver	*solver;
	btDiscreteDynamicsWorld				*world;
	//btDynamicsWorld						*world;

	qPhysicsShapeDrawer					*mShapeDrawer;
};

#endif