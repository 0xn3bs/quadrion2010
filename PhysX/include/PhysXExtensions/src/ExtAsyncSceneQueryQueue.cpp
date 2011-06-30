// This code contains NVIDIA Confidential Information and is disclosed to you 
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and 
// any modifications thereto. Any use, reproduction, disclosure, or 
// distribution of this software and related documentation without an express 
// license agreement from NVIDIA Corporation is strictly prohibited.
// 
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2011 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#include "ExtAsyncSceneQueryQueue.h"
#include "PxBatchQuery.h"


struct Ext::BatchQueryTask: public Ps::UserAllocated
{
	PX_INLINE BatchQueryTask(PxBatchQuery& sq) : batchQuery(sq) {}
	PxBatchQuery&	batchQuery;
};

Ext::AsyncSceneQueryQueue::AsyncSceneQueryQueue(const PxAsyncSceneQueryQueueDesc& desc)
{
	mTaskQueue = PX_NEW(TaskQueue)(desc.maxQueueSize);
	PX_ASSERT(mTaskQueue);
	setPriority(Ps::ThreadPriority::eNORMAL);
	setAffinityMask(desc.threadMask);
	start(desc.threadStackSize);
}

Ext::AsyncSceneQueryQueue::~AsyncSceneQueryQueue()
{
}

bool Ext::AsyncSceneQueryQueue::addBatchQuery(PxBatchQuery& sq, bool block)
{
	bool successAdd = false;
	BatchQueryTask* task = PX_NEW(BatchQueryTask)(sq);
	do 
	{
		mQueueMutex.lock();
		successAdd = mTaskQueue->pushBack(task);
		mQueueMutex.unlock();
		if(!successAdd && block)
			Ps::Thread::yield();
	}
	while (!successAdd && block);
	
	if(successAdd)
		mNewTaskSync.set();
	else
		PX_DELETE(task);
	
	return successAdd;
}

bool Ext::AsyncSceneQueryQueue::isQueueFinished(bool block)
{
	return mThreadDoneSync.wait(block ? -1 : 0);
}

PxU32 Ext::AsyncSceneQueryQueue::getSize()
{
	return mTaskQueue->size();
}

bool Ext::AsyncSceneQueryQueue::stallQueue(bool block)
{
	if(block)
		return mQueueMutex.lock();
	else
		return mQueueMutex.trylock();
}

void Ext::AsyncSceneQueryQueue::resumeQueue()
{
	mQueueMutex.unlock();
}

void Ext::AsyncSceneQueryQueue::release()
{
	mNewTaskSync.set();
	Ps::Thread::yield();
	isQueueFinished(true);
	signalQuit();
	mNewTaskSync.set();
	Ps::Thread::yield();
	waitForQuit();
	PX_DELETE_AND_RESET(mTaskQueue);
	delete this;
}

void Ext::AsyncSceneQueryQueue::execute()
{
	mThreadDoneSync.reset();
	while(true)
	{
		// check if there is at least one input, else goto sleep
		if(mTaskQueue->size())
		{
			Ps::Mutex::ScopedLock lock(mQueueMutex);
			
			BatchQueryTask* task = mTaskQueue->popFront();
			task->batchQuery.execute();
			PX_DELETE(task);
		}
		else
		{
			// wait till a new Task or the end of the world has come
			mThreadDoneSync.set();
			if(quitIsSignalled()) quit(); 
			mNewTaskSync.wait();
			mNewTaskSync.reset();
			mThreadDoneSync.reset();
		}
	}
}

/*PX_PHYSX_CORE_API*/ PxAsyncSceneQueryQueue* PxAsyncSceneQueryQueueCreate(const PxAsyncSceneQueryQueueDesc& desc)
{
	return PX_NEW(Ext::AsyncSceneQueryQueue)(desc);
}
