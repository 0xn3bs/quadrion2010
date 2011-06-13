////////////////////////////////////////////////////////////////////////////////////////////////
//
// 


#ifndef INI_H
#define INI_H

#include <string>
#include <vector>
#include <windows.h>
#include "hashtable.h"
#include "qrender.h"
#include "split.h"

// flags for initialization variables that represent low, medium, and high settings //
extern UCHAR INI_QUALITY_LOW;
extern UCHAR INI_QUALITY_MEDIUM;
extern UCHAR INI_QUALITY_HIGH;



static UCHAR iniQualityAssign(std::string qual)
{
	if(qual.compare("low") == 0)
		return INI_QUALITY_LOW;
	else if(qual.compare("medium") == 0)
		return INI_QUALITY_MEDIUM;
	else if(qual.compare("high") == 0)
		return INI_QUALITY_HIGH;
	else
		return INI_QUALITY_MEDIUM;	
}


class cINI
{
public:

	cINI();
//	static cINI* Instance();

	bool load( std::string fileName );
	void save( std::string fileName );
	void save();
	void clear() { iniVariables.clear(); }

	std::string getFileName() { return m_szFileName; }

	void LoadSettings(char* m_szFileName);

	std::string queryString(char *var) { return iniVariables[var]; }
	const char* queryChar(char *var) { return iniVariables[var].c_str(); }
	unsigned char queryUChar(char *var) { return iniQualityAssign(iniVariables[var]); }
	INT queryInt(LPSTR var) { return (INT)atoi(iniVariables[var].c_str()); }
	bool queryBool(char *var);

	const void setValue(char *var, char* val) { iniVariables[var] = val; }
	void setValue(char *var, bool val)  { if(val){ iniVariables[var] = "true"; } else {iniVariables[var] = "false";} }
	void setValue(char *var, int val)
	{
		char num[10];
		itoa(val, &num[0], 10);
		iniVariables[var] = num;
	}
	
	void setValue(char *var, std::string val)  { iniVariables[var] = val; }

private:
	std::string m_szFileName;
	HashTable <std::string> iniVariables;
};


#endif