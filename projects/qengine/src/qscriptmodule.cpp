//
//  qscriptmodule.cpp
//  qScript
//
//  Created by avansc on 6/7/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>

#include "qscriptmodule.h"

#include "angelscript.h"

using namespace std;


qscriptmodule::qscriptmodule()
{
}

qscriptmodule::qscriptmodule(asIScriptModule *_mod, std::string _name)
:	mod(_mod),
	name(_name)
{
}

qscriptmodule::~qscriptmodule()
{
}

int qscriptmodule::addSection(char *_script)
{
	int r = this->mod->AddScriptSection(this->name.c_str(), _script, strlen(_script));
	if( r < 0 ) 
	{
		printf("AddScriptSection() failed\n");
		return -1;
	}
	
	return r;
}

int qscriptmodule::addSectionFromFile(const char *_file)
{
	int r;
	
	// We will load the script from a file on the disk.
	FILE *f = fopen(_file, "rb");
	if( f == 0 )
	{
		printf("Failed to open the script file '%s'.\n", _file);
		//free(_file);
		return -1;
	}
	
	// Determine the size of the file	
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	
	// On Win32 it is possible to do the following instead
	// int len = _filelength(_fileno(f));
	
	// Read the entire file
    std::string script;
	script.resize(len);
	int c =	fread(&script[0], len, 1, f);
	fclose(f);
	//free(_file);
	
	if( c == 0 ) 
	{
		printf("Failed to load script file.\n");
		return -1;
	}
	
	//printf("%s\n", &script[0]);
	r = mod->AddScriptSection(this->name.c_str(), &script[0], len);
	if( r < 0 ) 
	{
		printf("AddScriptSection() failed\n");
		return -1;
	}
	printf("script loaded succesfully.\n");
	return 1;
}

int qscriptmodule::buildScript()
{
	
	//this->mod->
	int r = this->mod->Build();
	if( r < 0 )
	{
		printf("Build() failed\n");
		return -1;
	}
	return r;
}

void qscriptmodule::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
}