///////////////////////////////////////////////////////
//						TMS							 //
//				Thread Management System			 //
//                 By Renato Ciuffo					 //
///////////////////////////////////////////////////////

// TMS creates 1 thread on its constructor, said thread will be the manager and the the parent, 
// this thread will spawn 2 more (can be expanded later). 
// There is a function named AddTask(callback func) its overloaded to take in a  int, bool, 
// and a char as functions, AddTask will put the task in a buffer, the parent thread will 
// check this buffer whenever there is room in a child thread to do a task, 
// then itl assign a task to a thread, whitch will the run the callback function.
// AddTask returns the tasks ID.

// Avoid usage of #pragma once as it's non-standard.
// Use #ifndef/#define as an alternative for include protection.
// #pragma once

#ifndef __QTMS_H_
#define __QTMS_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <vector>
#include <sstream>
#include "qrender.h"
#include <string>


// Change this to change the amount of threads
const int TMS_MAX_THREAD_COUNT = 1;
const int TMS_MAX_THREAD_LOAD = 255;


enum TaskTypes
{
	TMS_TYPE_CALLBACK,
	TMS_TYPE_TEXTURELOAD,
	TMS_TYPE_NORMALMAPLOAD
};

struct TaskStruct
{
	TaskTypes		type;
	void			(*cbFunc)();
	void			*userData;
	std::string		sParam;
	unsigned int	uiParam;
	void			*misc;
	bool			done;
};


DWORD WINAPI ParentThread(LPVOID lpParam);

class QRENDEREXPORT_API cTMS
{
public:
	cTMS(void);
	~cTMS(void);
	
	static inline cTMS* Instance();

	DWORD	ThreadID[TMS_MAX_THREAD_COUNT];
	HANDLE	ThreadHandle[TMS_MAX_THREAD_COUNT];
	int		ThreadData[TMS_MAX_THREAD_COUNT];

	int		AddTask(void (*cbFunc)(), TaskTypes type, void *userData = NULL, unsigned int iparam = 0, std::string sparam = "", void *misc = NULL);
	void	ClearTasks();
	int		setThreadPriority(int priority);
	void	StopTMS();
	void	StartTMS();
	int		PauseThread();
	int		UnPauseThread();
	bool	IsRunning() { return running; }
	void	ShutDown();
	bool	IsShuttingDown() { return shuttingDown; };
	int		CheckLoad() { return (int)tasks.size()+1; };

	std::vector<TaskStruct*> tasks;

private:

	DWORD	threadId;
	HANDLE	threadHandle;
	int		threadData;
	int		priority;
	bool	shuttingDown;
	bool	paused;
	bool	running;
};


#endif /*__QTMS_H_*/