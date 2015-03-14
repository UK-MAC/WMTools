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


/** @mainpage WMToolsCPP Source Code Documentation
 *
 * WMTools is a suite of libraries and applications designed around memory tracing.
 * Originally a C project the newest incarnation is a C++ project for tighter integration.
 *
 *
 *
 * @section Tools
 * - WMTrace -> The basis library for memory tracing.
 * - WMAnalysis -> Post-process analysis engine for output files.
 * - WMModel -> An automated memory model generator.
 *
 * @section WMTrace
 *   WMTrace is a very simple library, based on Posix function interposition.
 *
 *   Designed for MPI applications the libraries start and stop is triggered by MPI_Init and MPI_Finalize respectivly.
 *
 *   The basis of the library is to collect information regarding the time, volume and location of memory allocations.
 *
 *	@par Stack Walking
 *   Location data is collected through stack tracing of the allocation event, through the libunwind directory.
 *
 * @par Compression / IO
 *   The full output is compressed through a zlib compression stream and output to file.
 *   On a one file per rank basis.
 */

#include "../include/WMTrace.h"

/**
 * WMTrace is the base class for the tracing element of WMTools.
 *
 * The class provides interface functions for memory events such as malloc, calloc, realloc and free.
 * The class provides it's own semephores to prevent the contamination of results.
 */
WMTrace::WMTrace() {

	/* Define data storage objects */
	stack_map = new StackMap();
	unwind = new StackUnwind();
	wmtrace_buffer = new TraceBuffer(stack_map);

	/* Default mpi values */
	wmtrace_rank = -1;
	wmtrace_comm = -1;

	/* Application Signals */
	wmtrace_started = 0;
	wmtrace_finished = 0;
	wmtrace_active = 0;

	/* Timers */
	wmtrace_app_stime = 0;

	/* Function Counters */
	wmtrace_malloc_counter = 0;
	wmtrace_calloc_counter = 0;
	wmtrace_realloc_counter = 0;
	wmtrace_free_counter = 0;

	time = new WMTimer();
	timer_counter=0;


	/* Control flags */
	complex_trace = false;
	post_process = false;
	post_process_graph = false;
	post_process_functions = false;
}

WMTrace::~WMTrace() {
	/* Free objects */
	delete stack_map;
	delete unwind;
	delete wmtrace_buffer;
	delete time;
}

void WMTrace::printTimer(){
	timer_counter++;
	/* Only print ever TIMERFREQUENCY invocations */
	if(timer_counter%TIMERFREQUENCY == 0){
		double elapsed_time;
		time->elapsedTime(&elapsed_time);
		wmtrace_buffer->addTimer(elapsed_time);
	}
}


void *WMTrace::traceMalloc(long size) {
	/* Call Timer */
	printTimer();

	int stackID = 0;
	void * ptr = __libc_malloc(size);
	if (complex_trace) {	//Stack Traversal
		vector<long> call_stack = unwind->fullUnwind();
		stackID = stack_map->addStack(call_stack);
	}

	wmtrace_buffer->addMalloc((long) ptr, getTimeDelta(), size, stackID);
	return ptr;
}

void * WMTrace::traceCalloc(long size, long count) {
	/* Call Timer */
	printTimer();


	int stackID = 0;
	if (complex_trace) {	//Stack Traversal
		vector<long> call_stack = unwind->fullUnwind();
		stackID = stack_map->addStack(call_stack);
	}

	void * ptr = __libc_calloc(size, count);

	wmtrace_buffer->addCalloc((long) ptr, getTimeDelta(), size *count, stackID);

	return ptr;
}

void *WMTrace::traceRealloc(void *ptrold, long size) {
	/* Call Timer */
	printTimer();


	void *ptr_new = __libc_realloc(ptrold, size);
	wmtrace_buffer->addRealloc((long) ptrold, (long) ptr_new, getTimeDelta(),
			size);
	return ptr_new;
}

void WMTrace::traceFree(void *ptr) {
	/* Call Timer */
	printTimer();


	wmtrace_buffer->addFree((long) ptr, getTimeDelta());
	__libc_free(ptr);
}
