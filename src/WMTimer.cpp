#include "../include/WMTimer.h"

WMTimer::WMTimer() {
	start_time_s = 0;
	start_time_us = 0;
	syncStart();
}

void WMTimer::syncStart(){
	//todTimer(&start_time);
	//microTimer(&start_time);
	gettimeofday(&old_t, (struct timezone *) 0);
	start_time_s = old_t.tv_sec;
	start_time_us = old_t.tv_usec;
}

void WMTimer::todTimer(double *et) {

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
	*et = time1.tv_sec + (time1.tv_nsec * 1.0e-9);

}

void WMTimer::microTimer(double *tm) {

	gettimeofday(&old_t, (struct timezone *) 0);

	*tm = (double)((old_t.tv_sec - start_time_s) + ((old_t.tv_usec - start_time_us)*1.0e-6));
	
}


void WMTimer::elapsedTime(double *et){
	double current_time;
	todTimer(&current_time);
	*et = (double)(current_time - (start_time_s + (start_time_us*1.0e-6)));
}
