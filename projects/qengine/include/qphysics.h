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

class qscriptengine;

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btVector3;
class CModelObject;
class btCompoundShape;

class QPHYSICSEXPORT_API qphysicsengine : public qobject
{
public:
	qphysicsengine();
	~qphysicsengine();

	void setGravity(vec3f const& _g);
	void setGravity(float const& x, float const& y, float const& z);

	btRigidBody *addBox(float mass, vec3f pos, vec3f size, CModelObject *_handle);
	btRigidBody *addCompoundShape(float mass, vec3f pos, vec3f size, btCompoundShape *shape);

	void step(float const& dt);

	void getAABB(btRigidBody *body, btVector3 &min, btVector3 &max);
	void updateCenterOfMassOffest(btRigidBody *body, CModelObject *mdl);

	virtual void REGISTER_SCRIPTABLES(qscriptengine *engine);
private:
	btBroadphaseInterface				*broadphase;
	btDefaultCollisionConfiguration		*collisionConfiguration;
	btCollisionDispatcher				*dispatcher;
	btSequentialImpulseConstraintSolver	*solver;
	btDiscreteDynamicsWorld				*world;
};

#endif