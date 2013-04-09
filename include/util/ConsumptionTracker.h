#ifndef CONSUMPTIONTRACKER
#define CONSUMPTIONTRACKER

#include "Util.h"

#include "malloc_obj.h"
#include "free_obj.h"
#include "ConsumptionGraph.h"
#include "FunctionSiteAllocation.h"

#include <map>
#include <set>

using namespace std;

/**
 * ConsumptionHWMTracker is an object class to sore all of the allocations and map them to their corresponding frees.
 *
 * This enables the class to track the memory consumption through time.
 * It records the HWM value, the time it occurred and the ID of the allocation causing it.
 *
 * HWM is determined upon a free even, to see if we are coming from a HWM point.
 * This means we must always check at the finish to see if the HWM occurred.
 */
class ConsumptionHWMTracker {
private:

	/** The HWM memory consumption (B) */
	long hwm;
	/** The time of the HWM (s)*/
	double hwm_time;
	/** The allocation ID of the HWM */
	long hwmID;

	/** The current memory consumption (B) */
	long curr_memory;
	/** The current elapsed time (s) */
	double curr_time;
	/** The current allocation ID */
	long currID;

	/** Should we produce a graph */
	bool graph;
	/** Should we store the data for samples rather than a graph */
	bool samples;

	/** Data structure to maintain a consumption graph */
	ConsumptionGraph *consumption;

	map<long, MallocObj> allocation_map;
	map<long, MallocObj>::iterator allocation_map_it;

	/**
	 * A function to check if we are at a HWM point, if so update the HWM variables.
	 */
	void checkHWM();

public:
	/**
	 * Constructor for the ConsumptionHWMTracker object.
	 * @param filename The name of the filename to store the graph in.
	 * @param graph If we should be storing the data for a graph.
	 * @param samples If the stored data will be used for samples rather than an actual graph.
	 */
	ConsumptionHWMTracker(string filename, bool graph = false, bool samples =
			false);

	/**
	 * Deconstructor for the ConsumptionHWMTracker object.
	 * Frees the allocated objects.
	 */
	~ConsumptionHWMTracker();

	/**
	 * Add an allocation event to the map.
	 * - Increase the current memory counter.
	 * - Increase the current time object.
	 * - Add the allocation object to the map.
	 *
	 * We assume that the pointer is unique so each map key has a single value.
	 *
	 * @param malloc The allocation object to add.
	 * @return The current allocation ID.
	 */
	long addAllocation(MallocObj& malloc);

	/**
	 * Add a free object to the map.
	 * - Update the memory HWM - as cheaper to do on a free than a malloc.
	 * - Increment the current time object.
	 * - Check to see if a matching allocation object exists.
	 * - Decrease current memory count.
	 * - Remove the allocation object from the map.
	 *
	 * @param free The free object.
	 * @return The current allocation ID.
	 */
	long addFree(FreeObj& malloc);

	/**
	 * Return a reference to the allocation object referred to by the pointer.
	 * Function returns NULL if allocation object not found.
	 *
	 * @param pointer The memory address of the allocated object.
	 * @return A pointer to the allocation object.
	 */
	MallocObj* getAllocation(long pointer);

	/**
	 * Return the memory HWM of the trace (so far) in bytes.
	 * Must first perform a check to see if the HWM has not been updated.
	 *
	 * @return The HWM in bytes
	 */
	long getHighWaterMarkMemory() {
		return hwm;
	}

	/**
	 * Return the time at which the HWM occurred in s.
	 *
	 * @return The time of the HWM in s
	 */
	double getHighWaterMarkTime() {
		return hwm_time;
	}

	/**
	 * Return the point at which the HWM occurred in terms of the allocation ID.
	 *
	 * @return The allocation ID of the point of HWM.
	 */
	long getHighWaterMarkID() {
		return hwmID;
	}

	/**
	 * Function to mark the end of trace reading.
	 * Chance to update the final variable as if a free.
	 */
	void finish();

	/**
	 * A function the return the amount of memory allocated by different function call stacks.
	 * We take the allocations currently live and group them by function site recording memory and qualtity.
	 *
	 * @return A set of the allocations grouped by call stack ID.
	 */
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> getFunctionBreakdown();

	/**
	 * A function to fetch the finish time of this trace.
	 *
	 * @return The time (s) for the last allocation in this trace.
	 */
	double getFinishTime() {
		return curr_time;
	}

	/**
	 * Using the data of all the allocation points reduce to a vector of samples points.
	 * Calculate time offset and record the memory consumption at each time.
	 * Samples normalised to the longest running time.
	 * Destructive function - on the sample vector.
	 *
	 * @param samples The number of sample points to generate.
	 * @param time The maximum trace runtime, to calculate sample points.
	 * @return The vector of memory consumption at each point.
	 */
	long *getHeatMapSamples(int samples, double time) {
		return consumption->getHeatMapSamples(samples, time);
	}

	/**
	 * Setter for the elf static memory.
	 * @param elf The static memory in bytes.
	 */
	void setElf(long elf) {
		consumption->setElf(elf);
	}

	/**
	 * Setter for the local HWM of this rank.
	 * @param thisHWM The HWM in bytes.
	 */
	void setLocalHwm(long localHWM) {
		consumption->setLocalHwm(localHWM);
	}

	/**
	 * Setter for the global HWM of this job.
	 * @param thisHWM The HWM in bytes.
	 */
	void setGlobalHwm(long globalHWM) {
		consumption->setGlobalHwm(globalHWM);
	}

	/**
	 * Setter for the rank of the job.
	 * @param rank The rank of this trace.
	 */
	void setRank(int rank) {
		consumption->setRank(rank);
	}

	/**
	 * A function to reset the current time, to the elapsed time as contained within a timer frame of the output.
	 */
	void updateElapsedTime(double elapsed_time){
		curr_time = elapsed_time;
	}

};

#endif
