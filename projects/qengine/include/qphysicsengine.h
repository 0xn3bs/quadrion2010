#ifndef _qPhysicsEngine_h
#define _qPhysicsEngine_h

#ifdef QRENDER_EXPORTS
	#define QPHYSICSENGINEEXPORT_API		__declspec(dllexport)
#else
	#define QPHYSICSENGINEEXPORT_API		__declspec(dllimport)
#endif

class btVector3;
class CModelObject;
class qRigidBody;
enum qPhysicsShape
{
	QSHAPE_BOX = 0,
	QSHAPE_SPHERE,
	QSHAPE_CAPSULE,
	QSHAPE_CONVEX_MESH
};

class qPhysicsEngine
{
public:
	virtual ~qPhysicsEngine(){}

	virtual void		init() = 0;
	virtual void		step(float dt) = 0;
	virtual qRigidBody*	addRigidBody(float mass, CModelObject *mdl, qPhysicsShape shape) = 0;
};

#endif