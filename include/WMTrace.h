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


#ifndef WMTRACE
#define WMTRACE

#include <sys/types.h>
#include <mpi.h>

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

#include "WMTimer.h"
#include "util/TraceBuffer.h"
#include "util/StackMap.h"
#include "util/CallStackTraversal.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
extern __typeof (malloc) __libc_malloc;
extern __typeof (calloc) __libc_calloc;
extern __typeof (realloc) __libc_realloc;
extern __typeof (free) __libc_free;
}

using namespace std;

using namespace std;

class WMTrace {
private:
	/* Data storage objects */
	TraceBuffer *wmtrace_buffer;
	StackUnwind *unwind;
	StackMap *stack_map;

	/* MPI Variables */
	int wmtrace_rank;
	int wmtrace_comm;

	/* Application Signals */
	int wmtrace_started;
	int wmtrace_finished;
	int wmtrace_active;

	/* Timer object */
	WMTimer *time;
	long timer_counter;

	/* Timers variables */
	double wmtrace_app_stime;
	double wmtrace_fun_stime;
	double wmtrace_fun_etime;

	/* Function Counters */
	long wmtrace_malloc_counter;
	long wmtrace_calloc_counter;
	long wmtrace_realloc_counter;
	long wmtrace_free_counter;

	/* Control Variables */
	bool complex_trace;
	bool post_process;
	bool post_process_graph;
	bool post_process_functions;

public:
	/**
	 * Constructor for the WMTrace object.
	 */
	WMTrace();

	/**
	 * De-constructor for the WMTrace object, frees memory allocated on the heap.
	 */
	~WMTrace();

	/**
	 * Set the internal MPI_COMM_RANK
	 * @param rank The new rank to store
	 */
	void setRank(int rank) {
		wmtrace_rank = rank;
	}

	/**
	 * Fetch the internal MPI_COMM_RANK
	 * @return The rank
	 */
	int getRank() {
		return wmtrace_rank;
	}

	/**
	 * Set the internal MPI_COMM_SIZE
	 * @param comm The size of the communicator MPI_COMM_WORLD to store
	 */
	void setCommSize(int comm) {
		wmtrace_comm = comm;
	}

	/**
	 * Fetch the internal MPI_COMM_SIZE
	 * @return The size of the communicator MPI_COMM_WORLD
	 */
	int getCommSize() {
		return wmtrace_comm;
	}

	/**
	 * A function to print the timer frame, every x number of allocations, if required.
	 * Uses the WMTimer object to extract the elapsed time, to correct any drift.
	 * Called on every event, but only output every TIMERFREQUENCY calls.
	 */
	void printTimer();

	/**
	 * Increment Malloc Counter
	 */
	void incrementMallocCounter() {
		wmtrace_malloc_counter++;
	}

	/**
	 * Increment Calloc Counter
	 */
	void incrementCallocCounter() {
		wmtrace_calloc_counter++;
	}

	/**
	 * Increment Realloc Counter
	 */
	void incrementReallocCounter() {
		wmtrace_realloc_counter++;
	}

	/**
	 * Increment Free Counter
	 */
	void incrementFreeCounter() {
		wmtrace_free_counter++;
	}

	/**
	 * Mark the tracer as having started
	 */
	void startTracing() {
		time->syncStart();
		time->todTimer(&wmtrace_fun_etime);
		wmtrace_started = 1;
	}

	/**
	 * Mark the tracer as having finished
	 */
	void finishTracing() {
		wmtrace_finished = 1;
		wmtrace_buffer->finishBuffer();
		//printf("Rank %d - Mallocs %ld Callocs %ld Re-allocs %ld Frees %ld \n", wmtrace_rank, WMTrace_malloc_counter, WMTrace_calloc_counter, WMTrace_realloc_counter, WMTrace_free_counter);
	}

	/**
	 * Determine if tracing is currently active
	 * If true do not proceed, otherwise do
	 * @return The active state of the library is active
	 */
	bool testActive() {
		return wmtrace_started == 0 || wmtrace_finished == 1
				|| wmtrace_active > 0;
	}

	/**
	 * Enter an active segment, increment blocking semaphore
	 */
	void enterActive() {
		wmtrace_active++; /*time->start();*/
		time->todTimer(&wmtrace_fun_stime);
	}

	/**
	 * Exit an active segment, decrement blocking semaphore
	 */
	void exitActive() { /*time->stop(); */
		wmtrace_active--;
		time->todTimer(&wmtrace_fun_etime);
	}

	/**
	 * Generate a timing delta from the last function call
	 * @return The difference between the memory function calls.
	 */
	float getTimeDelta() {
		return (float) (wmtrace_fun_stime - wmtrace_fun_etime);
	}


	/**
	 * Process a Malloc Event - Collect additional information + pass on to buffer
	 * @param[in] size The size of the allocation
	 *
	 * @return The memory allocated by the function
	 */
	void *traceMalloc(long size);


	/**
	 * Process a Calloc Event - Collect additional information + pass on to buffer
	 * @param[in] size The size of the allocation
	 * @param[in] count The number of objects of size size in this allocation
	 *
	 * @return The memory allocated by the function
	 */
	void *traceCalloc(long size, long count);

	/**
	 * Process a Realloc Event - Collect additional information + pass on to buffer
	 * @param[in] ptrold The existing memory pointer
	 * @param[in] size The size of the allocation
	 *
	 * @return The memory allocated by the function
	 */
	void *traceRealloc(void *ptrold, long size);

	/**
	 * Process a Free Event - Collect additional information + pass on to buffer
	 * @param[in] ptr The returned memory pointer
	 */
	void traceFree(void *ptr);

	/**
	 * Are we performing a complex trace.
	 * This consists of:
	 * - Call stack traversal.
	 * - Elf and dynamic libraries.
	 *
	 * @return If this is a complex trace.
	 */
	bool isComplexTrace() const {
		return complex_trace;
	}

	/**
	 * Set this to be a complex trace (or not), default to false.
	 *
	 * @param complexTrace If this should be a complex trace.
	 */
	void setComplexTrace(bool complexTrace) {
		this->complex_trace = complexTrace;
	}

	/**
	 * Should we post process after the execution.
	 * Produces HWM stats.
	 * Defaults to false.
	 *
	 * @return If we should post process after execution.
	 */
	bool isPostProcess() const {
		return post_process;
	}

	/**
	 * Set this to post process after execution.
	 *
	 * @param postProcess If we should post process after execution.
	 */
	void setPostProcess(bool postProcess) {
		this->post_process = postProcess;
	}

	/**
	 * Query if we should generate functions as a post process event.
	 */
	bool isPostProcessFunctions() const {
		return post_process_functions;
	}

	/**
	 * Define if we are to generate function lists after execution.
	 */
	void setPostProcessFunctions(bool postProcessFunctions) {
		this->post_process_functions = postProcessFunctions;
	}

	/**
	 * Query if we should generate graph scripts after execution.
	 */
	bool isPostProcessGraph() const {
		return post_process_graph;
	}

	/**
	 * Define if we are to generate graphs after execution.
	 */
	void setPostProcessGraph(bool postProcessGraph) {
		this->post_process_graph = postProcessGraph;
	}
};

#endif
