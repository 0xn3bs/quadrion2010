#ifndef __QTEXT_H_
#define __QTEXT_H_

#ifdef QRENDER_EXPORTS
	#define QTEXTEXPORT_API		__declspec(dllexport)
#else
	#define QTEXTEXPORT_API		__declspec(dllimport)
#endif


#include <windows.h>
#include <vector>
#include <string>


template <class T>
static T* toWideChar(const char* c)
{
	#ifdef UNICODE
		T* ret;
		if(c)
		{
			int nChars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, c, -1, NULL, 0);
			ret = new T[nChars];
			
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, c, -1, ret, nChars);
			return ret;
		}

		return ret;
	#else
		T* ret = c;
		return c;
	#endif
}


#ifdef UNICODE
	#define _QT(c) toWideChar <WCHAR>(c)
#else
	#define _QT(c) toWideChar <char>(c)
#endif

// This is pretty much a replica of the VB (and other languages) function Split()
QTEXTEXPORT_API std::vector <std::string> Split(const std::string& _separator, std::string _string);

QTEXTEXPORT_API std::vector <std::string> Split(const std::string& _separator, std::string _string, int _limit);

// 3rd Parameter returns the lengh
QTEXTEXPORT_API std::vector <std::string> SplitC(const std::string& _separator, std::string _string, int &length);


QTEXTEXPORT_API void ToLower(char* str);
QTEXTEXPORT_API void ToUpper(char* str);

QTEXTEXPORT_API bool IsLower(char c);
QTEXTEXPORT_API bool IsUpper(char c);

#endif
