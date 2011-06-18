#ifdef QRENDER_EXPORTS
	#define QEVENTLISTENEREXPORT_API		__declspec(dllexport)
#else
	#define QEVENTLISTENEREXPORT_API		__declspec(dllimport)
#endif

#ifndef _qeventlistener_h
#define	_qeventlistener_h

//#include "qComponent.h"

//#include "qEventHandler.h"
#include "qevent.h"
#include "qobject.h"

class QEVENTLISTENEREXPORT_API qeventlistener : public qobject//public qComponent
{
public:
	qeventlistener();
	virtual ~qeventlistener(){};
			
	virtual void ON_EVENT(const qevent &_evt);
			
	virtual event_type listener_type();
	virtual unsigned int listener_key();
			
	void set_key(unsigned int _key) { this->key = _key; }
			
protected:
	event_type type;
	unsigned int key;
};

#endif