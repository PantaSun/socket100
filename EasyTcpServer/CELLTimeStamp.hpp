#ifndef _CELL_TIME_STAMP_
#define _CELL_TIME_STAMP_
#include <chrono>
using namespace std::chrono;
class CELLTimeStamp
{
public:
	CELLTimeStamp() 
	{
		update();
	}
	~CELLTimeStamp(){}

	void update() {
		_timeStart = high_resolution_clock::now();
	}
	double getElapsedSecond() {
		return this->getElapsedTimeInMicroSec() * 0.000001;
	}
	double getElapsedTimeInMilliSec() {
		return this->getElapsedTimeInMicroSec() * 0.001;
	}
	long long getElapsedTimeInMicroSec() {
		return duration_cast<microseconds>(high_resolution_clock::now() - _timeStart).count();
	}

private:
	time_point<high_resolution_clock> _timeStart;
};

#endif // !_CELL_TIME_STAMP_
