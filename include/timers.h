#ifndef WMTIMER
#define WMTIMER

#include        <time.h>
#include        <sys/time.h>
#include        <sys/resource.h>
#include		<iostream>

using namespace std;

class WMTimer {

private:
	struct timeval old_t;

	timespec time1;

	double startTime;

public:
	/**
	 * Constructor for WMTimer sets a start time.
	 */
	WMTimer();

	/**
	 * Time of day timer returns time as a double since epoch.
	 * @param[out] et Pointer to double where the result should be stored.
	 */
	void todTimer(double *et);

	/**
	 * Time of day timer returns time as a float since program started.
	 * @param[out] et Pointer to float where the result should be stored.
	 */
	void micoTimer(float *tm);

};

#endif
