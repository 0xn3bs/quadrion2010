/*
* timer.cpp
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

#include "qtimer.h"
#include <stdlib.h>

void CTimer::Start()
{
#ifdef WIN32
	QueryPerformanceCounter(&StartCount);
#else
	gettimeofday(&StartCount, NULL);
#endif
	IsRunning = true;
}

void CTimer::Stop()
{
#ifdef WIN32
	QueryPerformanceCounter(&EndCount);
#else
	gettimeofday(&EndCount, NULL);
#endif
	IsRunning = false;
}

void CTimer::Reset()
{
	StartCount.LowPart = 0;
	StartCount.HighPart = 0;
	EndCount.LowPart = 0;
	EndCount.HighPart = 0;
	IsRunning = false;
}

double CTimer::GetElapsedMicroSec()
{
#ifdef WIN32
	if(IsRunning)
		QueryPerformanceCounter(&EndCount);

	StartTime = StartCount.QuadPart * (1000000.0 / Freq.QuadPart);
	EndTime = EndCount.QuadPart * (1000000.0 / Freq.QuadPart);
#else
	if (IsRunning)
		gettimeofday(&EndCount, NULL);

	StartTime = (StartCount.tv_sec * 1000000.0) + StartCount.tv_usec;
	EndTime = (EndCount.tv_sec * 1000000.0) + EndCount.tv_usec;
#endif
	return EndTime - StartTime;
}

double CTimer::GetElapsedMilliSec()
{
	return GetElapsedMicroSec() * 0.001;
}

double CTimer::GetElapsedSec()
{
	return GetElapsedMilliSec() * 0.001;
}

TIME CTimer::GetElapsed()
{
	TIME temp;
	temp.Microseconds = GetElapsedMicroSec();
	temp.Milliseconds = GetElapsedMilliSec();
	temp.Seconds = GetElapsedSec();
	temp.Minutes = GetElapsedSec() * 0.01666666;
	temp.Hours = temp.Minutes * 0.01666666;
	temp.Days = temp.Hours * 0.04166666;
	temp.Years = temp.Days * 0.00277777;

	return temp;
}
