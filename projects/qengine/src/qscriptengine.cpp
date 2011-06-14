//
//  qscriptengine.cpp
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <stdio.h>

#include "add_ons/scriptstring/scriptstring.h"
#include "qscriptengine.h"
#include "qscriptmodule.h"
#include "qscriptexec.h"

qscriptengine::qscriptengine()
{
    this->p_engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
    this->p_engine->SetMessageCallback(asFUNCTION(qscriptengine::MESSAGE_CALLBACK), 0, asCALL_CDECL);
    
    RegisterScriptString(this->p_engine);
    
    REGISTER_GLOBAL_FUNCTION(this, "void Print(string &in)", qscriptengine::DEBUG_PRINT);
}

qscriptengine::~qscriptengine()
{
}

qscriptmodule *qscriptengine::pGetScriptModule(std::string mod)
{
	asIScriptModule *p_mod = this->p_engine->GetModule(mod.c_str(), asGM_CREATE_IF_NOT_EXISTS);
	if(p_mod)
		return new qscriptmodule(p_mod, mod);
	else
		return NULL;
}

qscriptexec	*qscriptengine::pGetScriptExec(std::string mod, std::string func)
{
	asIScriptContext *ctx = this->p_engine->CreateContext();
	if(ctx->Prepare(this->p_engine->GetModule(mod.c_str())->GetFunctionIdByDecl(func.c_str())) < 0)
	{
		printf("Could not get qscriptexec\n");
		return NULL;
	}
	return new qscriptexec(ctx);
}

int qscriptengine::hasFunction(char *func)
{
	int len = this->p_engine->GetGlobalFunctionCount();
	for(int a = 0; a < len; a++)
	{
		if(strcmp_ws(func, (char*)this->p_engine->GetFunctionDescriptorById(this->p_engine->GetGlobalFunctionIdByIndex(a))->GetDeclaration()))
			return a;
	}
	return -1;
}

int qscriptengine::hasFunctionS(std::string &func)
{
	return this->hasFunction((char*)func.c_str());
}

int qscriptengine::hasProperty(char *func)
{
	int len = this->p_engine->GetGlobalPropertyCount();
	for(int a = 0; a < len; a++)
	{
		const char *name;
		this->p_engine->GetGlobalPropertyByIndex(a, &name, NULL, NULL);
		if(strcmp_ws(func, (char*)name) == 0) // changes strcmp to strcmp_ws, may cause issue.
			return a;
	}
	return -1;
}

int qscriptengine::hasPropertyS(std::string &func)
{
	return this->hasProperty((char*)func.c_str());
}

int qscriptengine::hasObjectType(const char *object)
{
	int len = this->p_engine->GetObjectTypeCount();
	for(int a = 0; a < len; a++ )
	{
		if(strcmp_ws((char*)object, (char*)this->p_engine->GetObjectTypeByIndex(a)->GetName()))
			return a;
	}
	return -1;
}
int qscriptengine::hasObjectTypeS(std::string &object)
{
	return this->hasObjectType(object.c_str());
}

int qscriptengine::objectHasMethod(const char *object ,const char *method)
{
	int o = this->hasObjectType(object);
	if(o >= 0)
	{
		int m = this->p_engine->GetObjectTypeByIndex(o)->GetMethodCount();
		for(int a = 0;a < m;a++)
		{
			if(strcmp_ws((char*)method, (char*)this->p_engine->GetObjectTypeByIndex(o)->GetMethodDescriptorByIndex(a, false)->GetDeclaration(false)))
				return a;
		}
	}
	return -1;
	
}

void qscriptengine::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
}

void qscriptengine::DEBUG_PRINT(std::string &str)
{
    std::cout << str;
}

void qscriptengine::MESSAGE_CALLBACK(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if( msg->type == asMSGTYPE_WARNING ) 
		type = "WARN";
	else if( msg->type == asMSGTYPE_INFORMATION ) 
		type = "INFO";
	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

bool qscriptengine::strcmp_ws(char *str1, char *str2)
{
	if(str1 == NULL || str2 == NULL) return false;
	
	char *t1 = str1;
	char *t2 = str2;
	
	while(*t1 != '\0' || *t2 != '\0')
	{
		while (*t1 == ' ') t1++;
		while (*t2 == ' ') t2++;
		if(*t1 != *t2) return false;
		if(*t1) t1++;
		if(*t2) t2++;
	}
	if(*t1 != *t2)
		return false;
	return true;
}