/*
* Copyright 2012 University of Warwick.
*
* This file is part of WMTools.
*
* WMTools is free software: you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the
* Free Software Foundation, either version 3 of the License, or (at your option)
* any later version.
*
* WMTools is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public License along with
* WMTools. If not, see http://www.gnu.org/licenses/. 
*/


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
