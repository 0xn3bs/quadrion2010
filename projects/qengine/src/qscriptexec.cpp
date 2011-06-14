//
//  qscriptexec.cpp
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

//#include "angelscript.h"
//#include "qscriptengine.h"
#include "qscriptexec.h"
#include "angelscript.h"

qscriptexec::qscriptexec()
{
	this->ctx = NULL;
	this->argc = 0;
}

qscriptexec::qscriptexec(asIScriptContext *_ctx)
:	ctx(_ctx),
    argc(0)
{
}

qscriptexec::~qscriptexec()
{
	this->ctx->Release();
}

int qscriptexec::exec()
{
	int r = this->ctx->Execute();
	
	if( r != asEXECUTION_FINISHED )
	{
		// The execution didn't finish as we had planned. Determine why.
		if( r == asEXECUTION_ABORTED )
			printf("The script was aborted before it could finish. Probably it timed out.\n");
		else if( r == asEXECUTION_EXCEPTION )
		{
			printf("The script ended with an exception.\n");
			
			// Write some information about the script exception
			int funcID = ctx->GetExceptionFunction();
			asIScriptFunction *func = this->ctx->GetEngine()->GetFunctionDescriptorById(funcID);
			printf("func: \n", func->GetDeclaration());
			printf("modl: \n", func->GetModuleName());
			printf("sect: \n", func->GetScriptSectionName());
			printf("line: \n", ctx->GetExceptionLineNumber());
			printf("desc: \n", ctx->GetExceptionString());
		}
		else
			printf("The script ended for some unforeseen reason (%d).\n", r);
	}
	else
	{
		// Retrieve the return value from the context
		//float returnValue = ctx->GetReturnFloat();
		//cout << "The script function returned: " << returnValue << endl;
	}
	return r;
}

int qscriptexec::reset()
{
	this->argc = 0;
	return this->ctx->Prepare(this->ctx->GetExceptionFunction());
}

void qscriptexec::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
	//REGISTER_CLASS(engine, "qscriptexec", qscriptexec);
}