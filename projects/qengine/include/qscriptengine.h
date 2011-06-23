//
//  qscriptengine.h
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _qscriptengine_h
#define _qscriptengine_h

#include <string>
#include <assert.h>
#include "../../../angelscript/include/angelscript.h"
#include "qscriptable.h"

#include <windows.h>
#include <fstream>
#include <sstream>

#ifdef QRENDER_EXPORTS
#define QSCRIPTEXPORT_API		__declspec(dllexport)
#define QSCRIPT_TEMPLATE
#else
#define QSCRIPTEXPORT_API		__declspec(dllimport)
#define QSCRIPT_TEMPLATE extern
#endif


void QSCRIPTEXPORT_API SCRIPT_ERROR(const char *format, ...);

///////////////////////
#define REGISTER_CLASS(ENGINE, NAME, CLASS)																									\
if(ENGINE->hasObjectType(NAME) < 0)																											\
{																																			\
assert(ENGINE->getEngine()->RegisterObjectType(NAME, sizeof(CLASS), asOBJ_REF) >= 0);													\
assert(ENGINE->getEngine()->RegisterObjectBehaviour(NAME, asBEHAVE_FACTORY, NAME" @f()", asFUNCTION(factory), asCALL_CDECL) >= 0);		\
\
assert(ENGINE->getEngine()->RegisterObjectBehaviour(NAME, asBEHAVE_FACTORY, NAME" @f(const "NAME"&in)", asFUNCTION(copyFactory), asCALL_CDECL) >= 0);		\
\
assert(ENGINE->getEngine()->RegisterObjectBehaviour(NAME, asBEHAVE_ADDREF, "void f()", asMETHOD(CLASS,addRef), asCALL_THISCALL) >= 0);	\
assert(ENGINE->getEngine()->RegisterObjectBehaviour(NAME, asBEHAVE_RELEASE, "void f()", asMETHOD(CLASS,release), asCALL_THISCALL) >= 0);\
}else																																		\
{																																			\
printf("Class name '%s' is already defined.\n", NAME);																					\
}																																			\


#define REGISTER_METHOD(ENGINE, NAME, CLASS, PROTO, FUNC)														\
if(ENGINE->objectHasMethod(NAME, PROTO) < 0)																	\
assert(ENGINE->getEngine()->RegisterObjectMethod(NAME, PROTO, asMETHOD(CLASS,FUNC), asCALL_THISCALL) >= 0);	\
else																											\
printf("Method name '%s' for class '%s' is already defined.\n", PROTO, NAME);								\


#define REGISTER_GLOBAL_FUNCTION(ENGINE, PROTO, FUNC)												\
if(ENGINE->hasFunction(PROTO) < 0)																	\
assert(ENGINE->getEngine()->RegisterGlobalFunction(PROTO, asFUNCTION(FUNC), asCALL_CDECL) >= 0);\
else																								\
printf("Global function '%s' is already defined.\n", PROTO);											\


#define REGISTER_GLOBAL_PROPERTY(ENGINE, DECLARATION, PROPERTY)						\
if(ENGINE->hasProperty(DECLARATION) < 0)											\
assert(ENGINE->getEngine()->RegisterGlobalProperty(DECLARATION, PROPERTY) >= 0);\
else																				\
printf("Global property '%s' is already defined.\n", DECLARATION);							\

#define REGISTER_TO_LIBRARY(ENGINE, NAME, FUNCTION)		\
ENGINE->getScriptLibrary()->add(NAME, FUNCTION);		\

///////////////////////

class qscriptmodule;
class qscriptexec;
class asIScriptEngine;
class asSMessageInfo;

class QSCRIPTEXPORT_API qscriptengine : public qscriptable<qscriptengine>
{
public:
    qscriptengine();
    ~qscriptengine();
    
    qscriptmodule	*pGetScriptModule(std::string mod);
    qscriptexec		*pGetScriptExec(std::string mod, std::string func);
    
    int	hasFunctionS(std::string &func);
    int	hasFunction(char *func);
    int	hasPropertyS(std::string &func);
    int	hasProperty(char *func);
    int	hasObjectType(const char *object);
    int	hasObjectTypeS(std::string &object);
    int	objectHasMethod(const char *object, const char *method);
    
    template<typename T>
    void registerScriptable();
    
    void REGISTER_SCRIPTABLES(qscriptengine *engine);
    
    asIScriptEngine *getEngine() { return this->p_engine; }
    
    static void	DEBUG_PRINT(std::string &str);
    static void	MESSAGE_CALLBACK(const asSMessageInfo *msg, void *param);
    static bool	strcmp_ws(char *str1, char *str2);
    
private:
    asIScriptEngine *p_engine;
};

template<typename T>
void qscriptengine::registerScriptable()
{
    T temp;
    temp.REGISTER_SCRIPTABLES(this);
}

QSCRIPT_TEMPLATE template class QSCRIPTEXPORT_API qscriptable<qscriptengine>;

#endif