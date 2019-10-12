#include <windows.h>
#include "stdafx.h"

double PCFreq = 0.0;
__int64 CounterStart = 0;
__int64 CounterLastFrame = 0;

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	return;

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

void ResetCounter()
{
	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
	CounterLastFrame = CounterStart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart)/PCFreq;
}

double GetCounterPassedTotal()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

double GetCounterPassed()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterLastFrame)/PCFreq;
}
