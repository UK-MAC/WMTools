#include "../include/timers.h"

WMTimer::WMTimer() {

	todTimer(&startTime);

}

void WMTimer::todTimer(double *et) {

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	*et = time1.tv_sec + time1.tv_nsec * 1.0e-9;

}

void WMTimer::micoTimer(float *tm) {

	gettimeofday(&old_t, (struct timezone *) 0);

	*tm = (float) ((double)(old_t.tv_sec + old_t.tv_usec * 1.0e-6) - startTime);
}



