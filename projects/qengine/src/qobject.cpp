/*
 *  qUtilLib
 *  qobject.cpp
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

#include "qscriptengine.h"
#include "qobject.h"

qobject::qobject()
:	_type(qobjectDefault)
{
}

qobject::qobject(qobjecttype type)
:	_type(type)
{
}
		
qobject::~qobject()
{
	std::map<std::string, qobject* >::iterator it;
			
	for(it = this->comps.begin();it != comps.end();it++)
		delete it->second;
}
		
void qobject::setRoot(qobject *_root)
{
	this->root = _root;
}
		
qobject *qobject::getRoot()
{
	return this->root;
}
		
qobject	*qobject::getComp(std::string comp)
{
	return this->comps[comp];
}
		
bool qobject::addComp(std::string comp, qobject *it)
{
	it->root = this;
	this->comps[comp] = it;
	return true;
}
		
		
void qobject::REGISTER_SCRIPTABLES(qscriptengine *engine)
{
	REGISTER_CLASS(engine, "qobject", qobject);
			
	int r = engine->getEngine()->RegisterObjectMethod("qobject", "qobject &opAssign(const qobject &in)", asMETHODPR(qobject, operator =, (const qobject&), qobject&), asCALL_THISCALL); assert( r >= 0 );
	
	REGISTER_METHOD(engine, "qobject", qobject, "bool addComp(string &comp, qobject &it)", addComp);
	REGISTER_METHOD(engine, "qobject", qobject, "qobject@ getComp(string &comp)", getComp);
			
	//REGISTER_GLOBAL_FUNCTION(engine, "qobject@ qComponent_convert(qobject@)", qComponent::convert);
}