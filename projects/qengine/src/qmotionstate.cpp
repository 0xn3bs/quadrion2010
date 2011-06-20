#include "qmotionstate.h"

#include "btBulletDynamicsCommon.h"
#include "qmodelobject.h"
#include "qmath.h"


qmotionstate::qmotionstate(const btTransform  &pos, CModelObject *_handle)
{
	this->p_Handle = _handle;
	this->mPos1 = pos;
}

void qmotionstate::setModelObject(CModelObject *_handle)
{
	this->p_Handle = _handle;
}

void qmotionstate::setWorldTransform(const btTransform &worldTrans)
{
	exit(-1);
	if(this->p_Handle == NULL) return;

	btQuaternion rot = worldTrans.getRotation();
	quat q(rot.x(), rot.y(), rot.z(), rot.w());
	mat4 id;
	QMATH_QUATERNION_MAKEMATRIX( id, q );
	this->p_Handle->SetModelOrientation( id );
	btVector3 pos = worldTrans.getOrigin();
	this->p_Handle->SetModelPos(vec3f(pos.x(), pos.y(), pos.z()));
}