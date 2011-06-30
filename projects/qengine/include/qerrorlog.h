// Avoid usage of #pragma once as it's non-standard.
// Use #ifndef/#define as an alternative for include protection.
// #pragma once
#ifndef _qErrorLog_h
#define _qErrorLog_h


#include <windows.h>
#include <fstream>
#include <sstream>


#ifdef QRENDER_EXPORTS
	#define QERRORLOGEXPORT_API		__declspec(dllexport)
#else
	#define QERRORLOGEXPORT_API		__declspec(dllimport)
#endif

//#pragma warning (disable : 4996)

#define LOG(msg) qErrorLog::Instance()->WriteError((msg));
#define QUIT_ERROR(msg, caption) qErrorLog::Instance()->quitError((msg), (caption));

class QERRORLOGEXPORT_API qErrorLog
{
private:
    qErrorLog();
    ~qErrorLog();

    inline char* CurrentTime() {
        GetSystemTime(&m_st);
        sprintf(m_szdate, "%02d/%02d/%d %02d:%02d:%02d.%03d | ", m_st.wMonth, m_st.wDay, m_st.wYear,
            m_st.wHour, m_st.wMinute, m_st.wSecond, m_st.wMilliseconds);
        return m_szdate;
    }  

    SYSTEMTIME		m_st;
    std::ofstream	m_ofelog;
    char			m_szdate[26];
public:
    static	qErrorLog* Instance();
    void	qErrorLog::WriteError(const char *format, ...);
	void	quitError(const char* mainText, const char* caption);
};

/////////////
// PROTOTYPES

void __cdecl odprintf(const char *format, ...);

// Wrapper for windows UNICODE versions now of MessageBox //
extern void quitError(const char* mainText, const char* caption);
extern void returnError(const char* mainText, const char* caption);
extern void systemToUser(const char* mainText, const char* caption, bool& val);

#endif /*__QMD3PARSER_H_*/
