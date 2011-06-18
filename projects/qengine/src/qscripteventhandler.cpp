/*
 *  qEventLib
 *  qNativeEventHandler.cpp
 *
 *	Copyright (c) 2001, AVS
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions are met:
 *	1.	Redistributions of source code must retain the above copyright
 *		notice, this list of conditions and the following disclaimer.
 *	2.	Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in the
 *		documentation and/or other materials provided with the distribution.
 *	3.	All advertising materials mentioning features or use of this software
 *		must display the following acknowledgement:
 *		This product includes software developed by the AVS.
 *	4.	Neither the name of the AVS nor the
 *		names of its contributors may be used to endorse or promote products
 *		derived from this software without specific prior written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY AVS ''AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL AVS BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "angelscript.h"
#include "qscriptexec.h"
#include "qscripteventhandler.h"
#include "qobject.h"
#include "qevent.h"

#include "qcamera.h"

#include "add_ons/scriptany/scriptany.h"

qscripteventhandler::qscripteventhandler()
:	exe(NULL)
{
}
		
void qscripteventhandler::ON_EVENT(const qevent &_evt)
{			
	this->ON_EVENT(_evt, ((qobject*)this->getRoot()));
}
		
void qscripteventhandler::ON_EVENT(const qevent &_evt, qobject *_obj)
{
	if(this->exe == NULL)
	{
		return;
	}
		
	printf("call\n");
			
	// Script call goes here.
	//this->on_event_ptr(_evt, _obj);
	
	//((CCamera*)_obj)->MoveCameraRelative(-0.1f, 0.0f, 0.0f);

	/*CScriptAny *any;
	extern qscriptengine *g_pScriptEngine;
	int typeID = g_pScriptEngine->getEngine()->GetTypeIdByDecl("CCamera@");
	any->Store((void*)&*/

	exe->ctx->SetArgObject(0, (void*)&_evt);
	exe->ctx->SetArgObject(1, (qobject*)_obj);
	exe->exec();
	exe->reset();			
}

void qscripteventhandler::set_script_exe(qscriptexec *_exe)
{
	this->exe = _exe;
}