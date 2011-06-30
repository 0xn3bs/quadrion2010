#include "qerrorlog.h"

qErrorLog* qErrorLog::Instance() {
    static qErrorLog pelog;
    return &pelog;
}

qErrorLog::~qErrorLog() {   
}

qErrorLog::qErrorLog() { 
}

void qErrorLog::WriteError(const char *format, ...) {
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
		strBuf <<  CurrentTime() << buf << std::endl;
		::OutputDebugStringA(strBuf.str().c_str());
}

void qErrorLog::quitError(const char* mainText, const char* caption)
{
	#if defined(UNICODE)
		MessageBoxA(NULL, LPCSTR(mainText), LPCSTR(caption), MB_OK | MB_ICONERROR);
		PostQuitMessage(0);
	#else
		MessageBox(NULL, mainText, caption, MB_OK | MB_ICONERROR);
		PostQuitMessage(0);
	#endif
}