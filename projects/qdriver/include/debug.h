// Avoid usage of #pragma once as it's non-standard.
// Use #ifndef/#define as an alternative for include protection.
// #pragma once
#ifndef __QDEBUG_H_
#define __QDEBUG_H_
#include <windows.h>
#include <fstream>


#pragma warning (disable : 4996)

////////// 
// CLASSES

// cErrorLog is a singleton that logs things in a text file. It opens and closes the file each
// time it prints something, to ensure nothing is lost in the event of a program crash.
// The member functions do the following:
//
// Instance() - Point of contact with the singleton.
// CurrentTime() returns a 26 character string with the current date and time.
// WriteError() prints the current date and time, followed by an error message.

class cErrorLog
{
private:
    cErrorLog();
    ~cErrorLog();

    inline char* CurrentTime() {
        GetSystemTime(&m_st);
        sprintf(m_szdate, "%02d/%02d/%d %02d:%02d:%02d.%03d | ", m_st.wMonth, m_st.wDay, m_st.wYear,
            m_st.wHour, m_st.wMinute, m_st.wSecond, m_st.wMilliseconds);
        return m_szdate;
    }  

    SYSTEMTIME m_st;
    std::ofstream m_ofelog;
    char m_szdate[26];
public:
    static cErrorLog* Instance();
    void __cdecl cErrorLog::WriteError(const char *format, ...);
};

/////////////
// PROTOTYPES

void __cdecl odprintf(const char *format, ...);

// Wrapper for windows UNICODE versions now of MessageBox //
extern void quitError(const char* mainText, const char* caption);
extern void returnError(const char* mainText, const char* caption);
extern void systemToUser(const char* mainText, const char* caption, bool& val);

#endif /*__QMD3PARSER_H_*/
