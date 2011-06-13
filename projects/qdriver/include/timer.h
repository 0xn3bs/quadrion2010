/*
* timer.h
*
* Created on: 25/01/2008
*     Author: Jonathan 'Bladezor' Bastnagel
*
* Copyright (C) <2008>  <Odorless Entertainment>
* 
*     This program is free software: you can redistribute it and/or modify
*     it under the terms of the GNU General Public License as published by
*     the Free Software Foundation, either version 3 of the License, or
*     (at your option) any later version.
* 
*     This program is distributed in the hope that it will be useful,
*     but WITHOUT ANY WARRANTY; without even the implied warranty of
*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*     GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public License
*     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TIMER_H_
#define TIMER_H_

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

struct STime
{
	double Years;
	double Days;
	double Hours;
	double Minutes;
	double Seconds;
	double Milliseconds;
	double Microseconds;
};

class Timer
{
public:
	Timer()
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

	~Timer()
	{
	}

	void Start();
	void Stop();

	double	GetElapsedMicroSec();
	double	GetElapsedMilliSec();
	double	GetElapsedSec();
	STime	GetElapsed();
	bool	IsRunning;

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

#endif /* TIMER_H_ */

