#include"TimeUtility.h"




TimeUtility::TimeUtility()
{
	PCFreq = 0.0;
	CounterStart = 0;
}

void TimeUtility::StartCounterMicro()
{
    // LARGE_INTEGER li;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    // if(QueryPerformanceFrequency(&li))
		// PCFreq = li.QuadPart/1000000.0;
    // QueryPerformanceCounter(&li);
    // CounterStart = li.QuadPart;
    CounterStart = now.tv_sec * 1000000.0 + now.tv_nsec / 1000.0;
}
double TimeUtility::GetCounterMicro()
{
    // LARGE_INTEGER li;
    // QueryPerformanceCounter(&li);
    // return double(li.QuadPart-CounterStart)/PCFreq;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return double(now.tv_sec * 1000000.0 + now.tv_nsec / 1000.0 - CounterStart);
}

void TimeUtility::StartCounterMill(){
    // LARGE_INTEGER li;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    // if(QueryPerformanceFrequency(&li))
		// PCFreq = li.QuadPart/1000000.0;
    // QueryPerformanceCounter(&li);
    // CounterStart = li.QuadPart;
    CounterStart = now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

double TimeUtility::GetCounterMill(){
    // LARGE_INTEGER li;
    // QueryPerformanceCounter(&li);
    // return double(li.QuadPart-CounterStart)/PCFreq;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return double(now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0 - CounterStart);
}