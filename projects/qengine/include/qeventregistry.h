
#ifdef QRENDER_EXPORTS
	#define QEVENTREGISTRYEXPORT_API		__declspec(dllexport)
#else
	#define QEVENTREGISTRYEXPORT_API		__declspec(dllimport)
#endif

#ifndef _qeventregistry_h
#define _qeventregistry_h

#include <map>
#include <list>
#include <vector>
#include <utility>
#include <string>

#include "qevent.h"

class qeventlistener;
class qeventhandler;

using namespace std;

//typedef multimap<unsigned int , multimap<unsigned int, qEventListener*> > listener_multimap;
typedef multimap<unsigned int, qeventlistener* > listener_multimap;
typedef multimap<unsigned int, qeventlistener* > monitor_multimap;
typedef multimap<unsigned int, multimap<unsigned int, qeventlistener* > > listener_multi_map;

const unsigned int EVENT_BUFFER_MAX = 10;

class QEVENTREGISTRYEXPORT_API qeventregistry
{
public:
	qeventregistry();
	bool push_event(const qevent &_evt);
			
	void process_events();
			
	bool register_pair(qeventlistener *L, qeventhandler *H, qobject *comp);

private:

	map<unsigned int, list<qeventlistener*> > testing;

	listener_multimap listeners;
	listener_multi_map listener_map;
	monitor_multimap monitor_map;
	//multimap<unsigned int, qEventListener*> listeners;
	vector<qevent> event_queue;
	qevent event_buffer[EVENT_BUFFER_MAX];
	unsigned int buffer_count;			
};

#endif