/*
 * ConsumptionGraph.h
 *
 *  Created on: 23 Jul 2012
 *      Author: ofjp
 */

#ifndef CONSUMPTIONGRAPH
#define CONSUMPTIONGRAPH

#include "Util.h"

#include <iostream>
#include <deque>
#include <utility>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

/**
 * Data structure to maintain the list of allocations to turn into a consumption graph.
 *
 * To reduce memory consumption and overheads we only store allocations summing to a difference of over GRAPHINTERVAL (1kb).
 *
 * For the outputted graph we use an even more coarse resolution to reduce the graph file size.
 * This resolution is based on a percentage of HWM, as this is not known during replay we store at a fine resolution, then coarsen later.
 */
class ConsumptionGraph {
private:

	/* Internal data structure to store the allocations */
	deque<pair<double, long> > consumption;
	/* Iterator over internal data structure */
	deque<pair<double, long> >::iterator consumption_it;

	/* Filename of the tracefile - used to generate the graph file */
	string outfile_name;

	/* Are we storing the consumption graph just for samples, or for the graph */
	bool samples;

	/* Store the rank ID of this job */
	int rank;

	/* Store the elf extracted static memory */
	long elf;

	/* Store the HWM of this rank */
	long local_HWM;

	/* Store the global HWM if possible */
	long global_HWM;

public:
	/**
	 * Constructor for the consumption graph
	 * @param filename The filename for the output graph.
	 * @param samples Are we storing the graph data just for samples.
	 */
	ConsumptionGraph(string filename, bool samples = false);

	/**
	 * Deconstructor for the ConsumptionGraph object.
	 * Frees any allocated memory.
	 */
	~ConsumptionGraph();

	/**
	 * Function to add a new allocation / de-allocation.
	 * Allocation at time of size memory.
	 * If memory is -ve then a deallocation.
	 * @param time The time at which the allocation occurred.
	 * @param memory The size -/+ve of the allocation
	 */
	void addAllocation(double time, long memory);

	/**
	 * Trigger the finish of the data structure occurring at time.
	 * Use time to calculate percentage times of points.
	 * We can get the total time from the time event of the last entry.
	 */
	void dumpGraphToFile();

	/**
	 * Using the data of all the allocation points reduce to a vector of samples points.
	 * Calculate time offset and record the memory consumption at each time.
	 * Samples normalised to the longest running time.
	 *
	 * @param samples The number of sample points to generate.
	 * @param time The maximum trace runtime, to calculate sample points.
	 * @return The vector of memory consumption at each point.
	 */
	long *getHeatMapSamples(int sampleCount, double time);

	/**
	 * Setter for the elf static memory.
	 * @param elf The static memory in bytes.
	 */
	void setElf(long elf) {
		this->elf = elf;
	}

	/**
	 * Setter for the local HWM of this rank.
	 * @param thisHWM The HWM in bytes.
	 */
	void setLocalHwm(long localHWM) {
		this->local_HWM = localHWM;
	}

	/**
	 * Setter for the global HWM of this job.
	 * @param thisHWM The HWM in bytes.
	 */
	void setGlobalHwm(long globalHWM) {
		this->global_HWM = globalHWM;
	}

	/**
	 * Setter for the rank of the job.
	 * @param rank The rank of this trace.
	 */
	void setRank(int rank) {
		this->rank = rank;
	}
};

#endif /* CONSUMPTIONGRAPH_H_ */
