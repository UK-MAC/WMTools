#ifndef WMTIMER
#define WMTIMER

#include        <time.h>
#include        <sys/time.h>
#include        <sys/resource.h>
#include		<iostream>
#include 		<iomanip>

using namespace std;

class WMTimer {

private:
	struct timeval old_t;

	timespec time1;

	double start_time;

public:
	/**
	 * Constructor for WMTimer sets a start time.
	 */
	WMTimer();

	/**
	 * A function to synchronise the start time constained within the object.
	 * The purpose of this functions allows the timer to be created, then the start time reset, when threads have synchronised.
	 */
	void syncStart();

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

	/**
	 * A function to calculate the elapsed time since the program started, as a double.
	 * This is used for the Timer frame for time correction.
	 * @param[out] et Pointer to the double object to store the elapsed time in.
	 */
	void elapsedTime(double *et);

};

#endif
