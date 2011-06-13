///////////////////////////////////////////////////////
//						TMS							 //
//				Thread Management System			 //
//                 By Renato Ciuffo					 //
///////////////////////////////////////////////////////

// TMS creates 1 thread on its constructor, said thread will be the manager and the parent, 
// this thread will spawn 2 more (can be expanded later). 
// There is a function named AddTask(callback func) its overloaded to take in a  int, bool, 
// and a char as functions, AddTask will put the task in a buffer, the parent thread will 
// check this buffer whenever there is room in a child thread to do a task, 
// then itl assign a task to a thread, whitch will the run the callback function.
// AddTask returns the tasks ID.

// Modified to only use one thread
//		- Renato | Dec 02, 2007


#include "qTMS.h"


cTMS::cTMS(void)
{
	// Spawn the Thread.
	threadHandle = CreateThread (
		0,
		0,
		ParentThread,
		&threadData,
		0,
		&threadId);

	priority = THREAD_PRIORITY_NORMAL;

	shuttingDown = false;
	paused = false;
	running = false;
}

cTMS::~cTMS(void)
{
	// This TerminateThread call is mostly for awkward shut downs where the ShutDown function isn't called
	TerminateThread(threadHandle, 0);
}

inline cTMS* cTMS::Instance()
{
	static cTMS m_pcTMS;
	return &m_pcTMS;
}

DWORD WINAPI ParentThread(LPVOID lpParam)
{
	cTMS *gTMS = cTMS::Instance();

	while(!cTMS::Instance()->IsShuttingDown())
	{
//		Sleep(100);
		
		if(!cTMS::Instance()->IsRunning())
			continue;

		// If there are no tasks, set the thread priority to IDLE, as though not to use unnecessary CPU cycles.
		if(gTMS->tasks.size() < 1)
		{
			gTMS->PauseThread();
			//gTMS->setThreadPriority(THREAD_PRIORITY_IDLE);
			continue;
		}

		// The task size is > 1 so set the priority back to normal
		gTMS->setThreadPriority(THREAD_MODE_BACKGROUND_BEGIN);
		for (int i = 0; i < gTMS->tasks.size(); ++i)
		{
			// Make sure that while we're in the middle of running tasks, the engine isnt shutting down
			if(cTMS::Instance()->IsShuttingDown())
				break;

			if(!gTMS->tasks[i])
				continue;

			if(!gTMS->tasks[i]->done)
			{
				if(gTMS->tasks[i]->type!=0)
				{
					switch(gTMS->tasks[i]->type)
					{
						case TMS_TYPE_CALLBACK:
						{
							if(!gTMS->tasks[i])
								continue;

							gTMS->tasks[i]->done = true;
							(*gTMS->tasks[i]->cbFunc)();
							delete gTMS->tasks[i];
							gTMS->tasks.erase(gTMS->tasks.begin() + i);
							break;
						}
						case TMS_TYPE_TEXTURELOAD:
						{
							if(!gTMS->tasks[i])
								continue;

							gTMS->tasks[i]->done = true;
							CQuadrionTextureObject* tex = (CQuadrionTextureObject*)gTMS->tasks[i]->userData;
							
							if(!tex->CreateTextureFromFile(gTMS->tasks[i]->uiParam))
							{
								//tex->CreateTextureFromFile(gTMS->tasks[i]->uiParam)
							}
							
							delete gTMS->tasks[i];
							gTMS->tasks.erase(gTMS->tasks.begin() + i);
							break;
						}

						case TMS_TYPE_NORMALMAPLOAD:
						{
							if(!gTMS->tasks[i])
								continue;

							gTMS->tasks[i]->done = true;
							CQuadrionTextureFile*	texFl = (CQuadrionTextureFile*)gTMS->tasks[i]->userData;
							std::string				newName;
							std::string				oldName = texFl->GetFileName(); // LoadTexture changes the fileName, so we need to store the old one here temporarily
							
							if(!texFl->LoadTexture(gTMS->tasks[i]->sParam.c_str(), texFl->GetFilePath().c_str()))
							{
								newName = gTMS->tasks[i]->sParam;
								newName.append(".dds");

								if(!texFl->LoadTexture(newName.c_str(), texFl->GetFilePath().c_str()))
								{
									newName = gTMS->tasks[i]->sParam;
									newName.append(".tga");

									if(!texFl->LoadTexture(newName.c_str(), texFl->GetFilePath().c_str()))
									{
										newName = gTMS->tasks[i]->sParam;
										newName.append(".jpg");

 										texFl->LoadTexture(newName.c_str(), texFl->GetFilePath().c_str());
									}
								}
							}

							// Set the filename back to the original so StreamTextureObject will get a proper handle
							newName = texFl->GetFileName();
							texFl->SetFileName(oldName);
							g_pRender->StreamTextureObject(*texFl, gTMS->tasks[i]->uiParam);
							texFl->SetFileName(newName);	// Now set it back

							
							delete gTMS->tasks[i];
							gTMS->tasks.erase(gTMS->tasks.begin() + i);
							break;
						}
					}

				} else {
					//g_pGUI->AddMessage("Type is 0!");
				}
			}
//			Sleep(25);	// 25 is best
		}
	}

	return 1;
}



int cTMS::AddTask(void (*cbFunc)(), TaskTypes type, void *userData, unsigned int uiparam, std::string sparam, void *misc)
{
	// Adds a task to the Task Buffer. Returns the tasks ID.
	TaskStruct *tmpTsk = new TaskStruct;
	tmpTsk->done =	false;
	tmpTsk->type =	type;
	tmpTsk->cbFunc = (*cbFunc);
	tmpTsk->userData = userData;
	tmpTsk->uiParam = uiparam;
	tmpTsk->sParam = sparam;
	tmpTsk->misc = misc;
	tasks.push_back(tmpTsk);

	// Make sure the thread is running
	if(paused)
		UnPauseThread();

	//g_pGUI->AddMessage("Added a task!");
	return 0;
}

// Pause the actual win32 thread
int cTMS::PauseThread()
{
	paused = true;
	return SuspendThread(threadHandle);
}

// Unpause the actual win32 thread
int cTMS::UnPauseThread()
{
	paused = false;
	return ResumeThread(threadHandle);
}

int cTMS::setThreadPriority(int newPriority)
{
	if(newPriority == priority)
		return false;

	SetThreadPriority(threadHandle, newPriority);
}

// Stop the TMS loop from running
void cTMS::StopTMS()
{
	running = false;
}

// Allow the TMS loop to run
void cTMS::StartTMS()
{
	running = true;
}

void cTMS::ShutDown()
{
	// This is the best way to shut TMS gracefully, set its shuttingDown variable to true and wait
	// a little bit as the code hits the block that checks for shuttingDown, THEN proceed to cleanning up.
	shuttingDown = true;
	Sleep(30);

	TerminateThread(threadHandle, 0);
}

void cTMS::ClearTasks()
{
	for (int i = 0; i < tasks.size(); ++i)
	{
		TaskStruct* task = tasks[i];

		tasks[i] = NULL;
		delete task;
	}

	tasks.clear();
}

