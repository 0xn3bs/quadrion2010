/*
* qtimer.h
*
* Created on: 25/01/2008
*     Author: Jonathan 'Bladezor' Bastnagel
*
* Copyright (C) <2008>  <Odorless Entertainment & Quadrion Software>
*/

#ifndef __QTIMER_H_
#define __QTIMER_H_

#ifdef WIN32
#include <windows.h>
#ifdef QRENDER_EXPORTS
#define QTIMEREXPORT_API		__declspec(dllexport)
#else
#define QTIMEREXPORT_API		__declspec(dllimport)
#endif
#else
#include <sys/time.h>
#include <unistd.h>
#endif


typedef struct QTIMEREXPORT_API _TIME
{
	double Years;
	double Days;
	double Hours;
	double Minutes;
	double Seconds;
	double Milliseconds;
	double Microseconds;
} TIME;

class QTIMEREXPORT_API CTimer
{
public:
	CTimer()
	{
#ifdef WIN32
		QueryPerformanceFrequency(&Freq);
		StartCount.QuadPart = 0;
		EndCount.QuadPart = 0;
#else
		StartCount.tv_sec = StartCount.tv_usec = 0;
		EndCount.tv_sec = EndCount.tv_usec = 0;
#endif	
		StartTime = 0;
		EndTime = 0;
	}

	~CTimer()
	{
	}

	void Start();
	void Stop();
	void Reset();

	double GetElapsedMicroSec();
	double GetElapsedMilliSec();
	double GetElapsedSec();
	TIME   GetElapsed();
	bool IsRunning;


protected:
private:

#ifdef WIN32
	LARGE_INTEGER Freq;
	LARGE_INTEGER StartCount;
	LARGE_INTEGER EndCount;
#else
	timeval StartCount;
	timeval EndCount;
#endif

	double StartTime;
	double EndTime;

};

#endif /*__QTIMER_H_*/

