#include "../include/WMTimer.h"

WMTimer::WMTimer() {
	start_time = 0.0;
	syncStart();
}

void WMTimer::syncStart(){
	todTimer(&start_time);
}

void WMTimer::todTimer(double *et) {

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	*et = time1.tv_sec + time1.tv_nsec * 1.0e-9;

}

void WMTimer::micoTimer(float *tm) {

	gettimeofday(&old_t, (struct timezone *) 0);

	*tm = (float) ((double)(old_t.tv_sec + old_t.tv_usec * 1.0e-6) - start_time);
}


void WMTimer::elapsedTime(double *et){
	double current_time;
	todTimer(&current_time);
	*et = (double)current_time - start_time;
}
