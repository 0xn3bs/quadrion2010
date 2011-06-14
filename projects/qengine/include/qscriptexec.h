//
//  qscriptexec.h
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _qscriptexec_h
#define _qscriptexec_h

#include <stdio.h>
//#include <angelscript.h>

#include "qscriptable.h"

#ifdef QRENDER_EXPORTS
#define QSCRIPTEXPORT_API		__declspec(dllexport)
#define QSCRIPT_TEMPLATE
#else
#define QSCRIPTEXPORT_API		__declspec(dllimport)
#define QSCRIPT_TEMPLATE extern
#endif

class asIScriptContext;
class qscriptengine;

enum QSCRIPTEXPORT_API AS_ARG_TYPE { AS_BYTE = 0, AS_WORD, AS_DWORD, AS_QWORD, AS_FLOAT, AS_DOUBLE, AS_ADDRESS, AS_OBJECT };

class QSCRIPTEXPORT_API qscriptexec : public qscriptable<qscriptexec>
{
public:
    qscriptexec();
    qscriptexec(asIScriptContext *_ctx);
    virtual ~qscriptexec();
    
    int exec();
    int reset();
    
    void REGISTER_SCRIPTABLES(qscriptengine *engine);
    
    //private:
    asIScriptContext *ctx;
    int argc;
};

QSCRIPT_TEMPLATE template class QSCRIPTEXPORT_API qscriptable<qscriptexec>;

#endif