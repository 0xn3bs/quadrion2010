//
//  qscriptmodule.h
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef _qscriptmodule_h
#define _qscriptmodule_h

#include <string>

#include "qscriptable.h"

#ifdef QRENDER_EXPORTS
#define QSCRIPTEXPORT_API		__declspec(dllexport)
#define QSCRIPT_TEMPLATE
#else
#define QSCRIPTEXPORT_API		__declspec(dllimport)
#define QSCRIPT_TEMPLATE extern
#endif

class asIScriptModule;

class QSCRIPTEXPORT_API qscriptmodule : public qscriptable<qscriptmodule>
{
public:
    
    qscriptmodule();
    qscriptmodule(asIScriptModule *_mod, std::string _name);
    virtual ~qscriptmodule();
    
    int addSection(char *_script);
    int addSectionFromFile(const char *_file);
	int addSectionFromFile(const std::string& file, const std::string& path);
    
    int buildScript();
    
    void REGISTER_SCRIPTABLES(qscriptengine *engine);
    
private:
    asIScriptModule *mod;
    std::string name;
};

QSCRIPT_TEMPLATE template class QSCRIPTEXPORT_API qscriptable<qscriptmodule>;

#endif