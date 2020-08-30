#include <time.h> 
#include<iostream>


#ifndef TIME_UTILITY_H
#define TIME_UTILITY_H

class TimeUtility{
private:

	double PCFreq;
	long CounterStart;

public:
	TimeUtility();

	void StartCounterMicro();
	double GetCounterMicro();

	void StartCounterMill();
	double GetCounterMill();
};

#endif