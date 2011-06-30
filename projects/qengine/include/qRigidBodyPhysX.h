#ifndef _qRigidBodyPhysX_h
#define _qRigidBodyPhysX_h

#ifdef QRENDER_EXPORTS
	#define QRIGIDBODYPHYSXEXPORT_API		__declspec(dllexport)
#else
	#define QRIGIDBODYPHYSXEXPORT_API		__declspec(dllimport)
#endif

#include "qmath.h"
#include "qRigidBody.h"

class PxRigidActor;
class PxRigidDynamic;

class QRIGIDBODYPHYSXEXPORT_API qRigidBodyPhysX : qRigidBody
{
public:
	qRigidBodyPhysX(PxRigidDynamic *_body);
	virtual ~qRigidBodyPhysX(){};

	virtual void getPose(mat4& pose);

private:
	PxRigidActor *body;
};

#endif