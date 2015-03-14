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


#ifndef TRACEREADER
#define TRACEREADER

#include "Util.h"

#include "Decompress.h"
#include "FrameData.h"
#include "ConsumptionTracker.h"
#include "ConsumptionGraph.h"
#include "StackProcessingMap.h"
#include "FunctionSiteAllocation.h"
#include "FunctionMap.h"
#include "RunData.h"
#include "malloc_obj.h"
#include "free_obj.h"

#include <iostream>
#include <assert.h>
#include <set>
#include <vector>

using namespace std;

/**
 * Class to read trace files.
 * Operates on two mode:
 * - Simple - Just get HWM
 * - Complex - Generate graphs
 * When in Simple mode we want to skip the ELF processing, Function processing and the Stack processing to make HWM reporting as fast as possible.
 * For complex we need to read all of the other tags.
 * We assume we are in simple mode unless told otherwise.
 * Complex had three optional modes:
 * - Print consumption graph
 * - Print HWM functional breakdown
 * - Print HWM live allocation data
 *
 */
class TraceReader {
private:
	ZlibDecompress *zlib_decomp;
	FrameData *frame_data;
	ConsumptionHWMTracker *hwm_tracker;
	FunctionMap * f_map;
	StackProcessingMap * stack_map;

	RunData *runData;

	/* Are we in simple or complex mode */
	bool complex;
	/* Print the consumption allocation graph */
	bool consumption_graph;
	/* Print the functional breakdown at HWM */
	bool function_graph;
	/* Print the allocation breakdown at HWM */
	bool allocation_graph;
	/* Print the samples consumption points rather than a graph */
	bool samples;

	/* The ID of the allocation to search for */
	long searchID;

	/* The Time in s to search for */
	double searchTime;
	
	/* Variable to mark the quick escape of the tracer */
	bool quick_finish;
	/* Store the elf recorded static memory */
	long static_mem;

	void read();

	/**
	 * Read a Malloc from the Buffer
	 * Takes the form of:
	 * 'M'<(long)Address><(float)Timestamp as delta><(long)Alloc size><(int)StackID>
	 *
	 *  Form a Malloc object (MallocObj) and add it to the storage structure.
	 *
	 *  @return The allocation ID of the event.
	 */
	long processMalloc();

	/**
	 * Read a Calloc from the Buffer
	 * Takes the form of:
	 * 'C'<(long)Address><(float)Timestamp as delta><(long)Alloc size><(int)StackID>
	 *
	 * Form a Malloc object (MallocObj) and add it to the storage structure.
	 * At this point the system does not differentiate between Malloc and Calloc.
	 *
	 *  @return The allocation ID of the event.
	 */
	long processCalloc();

	/**
	 * Read a Realloc from the Buffer
	 * Takes the form of:
	 * 'R'<(long)Old address><(long)New address><(float)Timestamp as delta><(long)Alloc size>
	 *
	 * Process a Realloc as if it was a Free and a Malloc.
	 * - Find the original allocation
	 * - If not found treat realloc as malloc, with -1 as stack ID. Add to the storage structure.
	 * - Otherwise form a new malloc object (MallocObj) from the old data.
	 * - Form a free object (FreeObj) from the malloc data.
	 * - Execute the free.
	 * - Execute the allocation.
	 *
	 * Must be this way around to ensure only a single entry for a key in the map, should the realloc return the same address.
	 *
	 *  @return The allocation ID of the last of the events as a realloc (if matched) represents a free and a malloc.
	 */
	long processRealloc();

	/**
	 * Read a Free from the Buffer
	 * Takes the form of:
	 * 'F'<(long)Address><(float)Timestamp as delta>
	 *
	 * Make a free object (FreeObj) from the data and add to the storage structure.
	 *
	 *  @return The allocation ID of the event.
	 */
	long processFree();

	/**
	 * Read in an Events frame, which will contain allocation events.
	 * Contains a collection of malloc / calloc / realloc and free frames.
	 * Takes the form of:
	 * 'E'<(long) Data size ><Malloc / Calloc / Realloc / Free >...
	 */
	void processEvents();

	/**
	 * Read a stack frame, which will contain a number of call stacks.
	 * Takes the form of:
	 * 'S'<(long) Frame Size><(int) Number of call stacks in frame>
	 * 		< <(int) Stack ID><(int) Function Count>
	 * 			< <(long) function address> <(long) function address> ...> >
	 *
	 */
	void processStacks();

	/**
	 * Read an Elf Frame
	 * Format of ELF Frame is:
	 *  'E' < (long)Elf static memory > < (int)Elf function count > < (long)Frame size (b) >
	 * 		<(long)Function start><(int)Function name length><Function Name>
	 * 		<(long)Function start><(int)Function name length><Function Name>
	 */
	void processElf();

	/**
	 * Read an Cores Frame
	 * Format of Cores Frame is:
	 *  'C' < (long)Frame size > < (int) Rank > < (int) Comm size ><(int) Name length><(char *) Name>
	 */
	void processCores();

	/**
	 * Read a Function Frame
	 * Format of Cores Frame is:
	 *  'V' < (long)Frame size > < (int) Function count >
	 *  	< (long) Address start ><(long) Address end><(int) Name length><(char *) Name>
	 *  	< (long) Address start ><(long) Address end><(int) Name length><(char *) Name>
	 */
	void processFunctions();

	/**
	 * A function to check the result of an allocation (/free) against the search criteria.
	 * We only perform the check if we are in a complex analysis - Otherwise return false.
	 * @param id The id of the newest allocation to check against.
	 * @return If the search term was true, in which case we can stop execution.
	 */
	bool checkIDSearch(long id);


	/**
	 * A function to read a timer frame.
	 * A timer frame contains the elapsed time since the program started.
	 * This is used to correct the timer drift introduced to excessive compression of tiny numbers.
	 * 'T'<(double) elapsed time>
	 *
	 */
	void processTimer();

public:
	/**
	 * Constructor for the TraceReader object.
	 * Has four optional arguments, which if not specified will assume that we are doing a simple memory trace.
	 * If they are specified we perform a complex trace with additional functionality.
	 * Decides if this is complex if any of the values are set.
	 *
	 * @param filename The name of the file we should be reading in, or auto-generate if not provided.
	 * @param consumptionGraph Should we produce a temporal consumption graph
	 * @param functionGraph Should we produce a functional breakdown at high water mark
	 * @param allocationGraph Should we produce an allocation breakdown at high water mark
	 * @param samples Should we collect point information for heat map samples
	 * @param searchID Specify a an allocation ID to search for - used to support multipass searches
	 * @param searchTime Stop at a specific time
	 */
	TraceReader(string filename = "", bool consumptionGraph = false,
			bool functionGraph = false, bool allocationGraph = false,
			bool samples = false, long searchID = -1, double searchTime = -1);

	/**
	 * Deconstructor for the TraceReader object.
	 * Frees as much memory as possible.
	 */
	~TraceReader();

	/**
	 * Fetch the memory consumption high water mark
	 * @return The memory high water mark in bytes
	 */
	long getHWMMemory() {
		return hwm_tracker->getHighWaterMarkMemory();
	}

	/**
	 * Fetch the time the memory high water mark occurred
	 * @return The time from program start until the point of high water mark
	 */
	double getHWMTime() {
		return hwm_tracker->getHighWaterMarkTime();
	}

	/**
	 * Fetch the allocation ID at which the memory high water mark occurred
	 * @return The allocation ID of the point of high water mark
	 */
	long getHWMID() {
		return hwm_tracker->getHighWaterMarkID();
	}

	long getCurrMemory(){
		return hwm_tracker->getCurrMemory();
	}


	double getCurrTime(){
		return hwm_tracker->getCurrTime();
	}


	/**
	 * A function the return the amount of memory allocated by different function call stacks.
	 * We take the allocations currently live and group them by function site recording memory and qualtity.
	 *
	 * @return A set of the allocations grouped by call stack ID.
	 */
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> getFunctionBreakdown() {
		return hwm_tracker->getFunctionBreakdown();
	}

	/**
	 * A function to fetch the composite functions of a call stack in terms of their strings.
	 *
	 * @param id The ID of the call stack to represent.
	 *
	 * @return A queue of strings of the function names of this call stack.
	 */
	vector<string> getCallStack(int id);

	/**
	 * A function to return the function map - to be used for mapping purposes.
	 * @return The FunctionMap object.
	 */
	FunctionMap* getFunctionMap() {
		return f_map;
	}

	/**
	 * Return the call stack map object (StackProcessingMap).
	 *
	 * @return The StackProcessingMap object.
	 */
	StackProcessingMap *&getCallStacks() {
		return stack_map;
	}

	/**
	 * Return the run data about the trace.
	 * Will return null if no run data has been found.
	 *
	 * @return The run data for this trace.
	 */
	RunData *&getRunData() {
		return runData;
	}

	/**
	 * A function to fetch the finish time of this trace.
	 *
	 * @return The time (s) for the last allocation in this trace.
	 */
	double getFinishTime() {
		return hwm_tracker->getFinishTime();
	}

	/**
	 * Using the data of all the allocation points reduce to a vector of samples points.
	 * Calculate time offset and record the memory consumption at each time.
	 * Samples normalised to the longest running time.
	 *
	 * @param samples The number of sample points to generate.
	 * @param time The maximum trace runtime, to calculate sample points.
	 * @return The vector of memory consumption at each point.
	 */
	long *getHeatMapSamples(int samples, double time) {
		return hwm_tracker->getHeatMapSamples(samples, time);
	}

	/**
	 * A function to return the elf recorded static memory from the binary.
	 * @return The static memory consumed within the binary in bytes.
	 */
	long getStaticMem() {
		return static_mem;
	}
};

#endif
