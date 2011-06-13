#include <iostream>
#include <windows.h>
#include "cINI.h"
#include "hashtable.h"


unsigned char INI_QUALITY_LOW		= 0x01;
unsigned char INI_QUALITY_MEDIUM	= 0x02;
unsigned char INI_QUALITY_HIGH		= 0x03;

void ToLower(char* _szData);
void ToUpper(char* _szData);
bool IsUpper(const char c);
bool IsLower(const char c);

void ToLower(char* _szData)
{
	for(unsigned int i = 0; i < strlen(_szData); i++)
		if(IsUpper(_szData[i]))
			_szData[i] += 32;
}
void ToUpper(char* _szData)
{
	for(unsigned int i = 0; i < strlen(_szData); i++)
		if(IsLower(_szData[i]))
			_szData[i] -= 32;
}
bool IsUpper(const char c)
{
	return (c > 64 && c < 91);
}

bool IsLower(const char c)
{ 
	return (c > 96 && c < 123);
}

cINI::cINI()
{
	m_szFileName = "";
}
/*
cINI* cINI::Instance()
{
	static cINI m_pINI;
	return &m_pINI;
}
*/

void cINI::LoadSettings(char* szFileName)
{
}

bool cINI::load( std::string fileName )
{
	std::ifstream file (fileName.c_str());	// Open file
	std::string szLine;

	if (file.is_open())
	{
		while (!file.eof() )	// Loop untill you reach eof (end of file)
		{
			getline (file,szLine);

			// Make sure the line is long enough
			if(szLine.size() <= 0)
				continue;

			// Check if the first character is a ';', therefore making it a comment
			if(strcmp(szLine.substr(0, 1).c_str(), ";") == 0)
				continue;

			int eqLoc = szLine.find("=");

			// if its not an error / if a '=' exists in the line
			if(eqLoc != -1)
			{
				std::vector <std::string> lineSplit;
				lineSplit = Split("=", szLine,1);

				std::string var = lineSplit[0];
				std::string value = lineSplit[1];
				iniVariables[var] = value;
			} else {
				continue;
			}
		}
	}

	else
		return false;

	m_szFileName = fileName;
	file.close();

	return true;
}

void cINI::save()
{
	save(m_szFileName);
}

///////////////////////////
// Saves the data to a file
void cINI::save( std::string fileName )
{
	std::ofstream file;
	file.open(fileName.c_str());

	for(int i = 0; i < iniVariables.size(); i++)
		file << iniVariables.getIndexName(i) << "=" << iniVariables[i] << "\n";

	file.close();
}

/////////////////////////////////
// Queries the ini for a variable
bool cINI::queryBool(char *var)
{
	char *lcase = (char *)iniVariables[var].c_str();
	ToLower(lcase);
	if(strcmp(lcase, "true") == 0)
		return true;
	else
		return false;
}


