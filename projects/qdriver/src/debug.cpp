#include "debug.h"
#include <sstream>



////////////////
// CLASS MEMBERS

cErrorLog* cErrorLog::Instance() {
    static cErrorLog pelog;
    return &pelog;
}

cErrorLog::~cErrorLog() {   
}

cErrorLog::cErrorLog() { 
}

void __cdecl cErrorLog::WriteError(const char *format, ...) {
	char buf[32767], *p = buf;
	va_list args;

       va_start(args, format);
       p += _vsnprintf(p, sizeof buf - 1, format, args);
       va_end(args);

        while ( p > buf  &&  isspace(p[-1]) )
                *--p = '\0';

        *p++ = '\n';
        *p   = '\0';
         
        //m_ofelog.open("error.log", std::ios::out | std::ios::app);
        //m_ofelog << CurrentTime() << buf;
       // m_ofelog.close();

		std::stringstream strBuf;
		strBuf << "DEBUG: " << buf << std::endl;
		OutputDebugString(strBuf.str().c_str());
}

////////////
// FUNCTIONS

void __cdecl odprintf(const char *format, ...) {
char buf[4096], *p = buf;
va_list args;

        va_start(args, format);
        p += _vsnprintf(p, sizeof buf - 1, format, args);
        va_end(args);

        while ( p > buf  &&  isspace(p[-1]) )
                *--p = '\0';

        *p++ = '\r';
        *p++ = '\n';
        *p   = '\0';

        OutputDebugString(buf);
}

void quitError(const char* mainText, const char* caption)
{
	#if defined(UNICODE)
		MessageBoxA(NULL, LPCSTR(mainText), LPCSTR(caption), MB_OK | MB_ICONERROR);
		PostQuitMessage(0);
	#else
		MessageBox(NULL, mainText, caption, MB_OK | MB_ICONERROR);
		PostQuitMessage(0);
	#endif
}

void returnError(const char* mainText, const char* caption)
{
	cErrorLog::Instance()->WriteError(mainText);
}

void systemToUser(const char* mainText, const char* caption, bool& val)
{
	#if defined(UNICODE)
		(MessageBoxA(NULL, LPCSTR(mainText), LPCSTR(caption), MB_YESNO | MB_ICONQUESTION) == IDNO) ? val = false : val = true;
	#else
		(MessageBox(NULL, mainText, caption, MB_YESNO | MB_ICONQUESTION) == IDNO) ? val = false : val = true;
	#endif
}
