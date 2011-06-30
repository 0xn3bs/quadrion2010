#ifndef _qRigidBody_h
#define _qRigidBody_h

#ifdef QRENDER_EXPORTS
	#define QRIGIDBODYEXPORT_API		__declspec(dllexport)
#else
	#define QRIGIDBODYEXPORT_API		__declspec(dllimport)
#endif

#include "qmath.h"

class QRIGIDBODYEXPORT_API qRigidBody
{
public:
	qRigidBody(){};
	virtual ~qRigidBody(){};

	virtual void getPose(mat4& pose) = 0;

private:
};

#endif