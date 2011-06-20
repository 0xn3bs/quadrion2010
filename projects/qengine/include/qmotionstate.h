#ifdef QRENDER_EXPORTS
	#define QMOTIONSTATEEXPORT_API		__declspec(dllexport)
#else
	#define QMOTIONSTATEEXPORT_API		__declspec(dllimport)
#endif

#ifndef _qmotionstate_h
#define _qmotionstate_h

#include "btBulletDynamicsCommon.h"
#include "qmodelobject.h"

class QMOTIONSTATEEXPORT_API qmotionstate : public btMotionState
{
public:
	qmotionstate(const btTransform &pos, CModelObject *_handle);
	virtual ~qmotionstate(){};

	void setModelObject(CModelObject *_handle);

	virtual void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = mPos1;
    }

	virtual void setWorldTransform(const btTransform &worldTrans);

protected:
	CModelObject *p_Handle;
	btTransform mPos1;
};

#endif