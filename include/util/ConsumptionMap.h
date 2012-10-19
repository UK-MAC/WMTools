/*
 * ConsumptionMap.h
 *
 *  Created on: 3 Aug 2012
 *      Author: ofjp
 */

#ifndef CONSUMPTIONMAP_H_
#define CONSUMPTIONMAP_H_

#include <vector>

#include "TraceReader.h"
#include "FunctionSiteAllocation.h"
#include "CallStackMapper.h"

#include "stdio.h"
#include "cmath"

using namespace std;

/**
 * ConsumptionMap is an object to map memory consumption between HWM points in traces.
 *
 * The idea is to map stacks to global IDs and then map the memory consumption between local ID's to global ID's.
 * This is done by iterating through the local memory consumption and mapping it.
 */
class ConsumptionMap {

private:
	vector<vector<long> > memory_map;
	vector<vector<FunctionSiteAllocation *> > fsa_map;
	CallStackMapper *stacks;
	/* Number of trace files */
	int trace_count;
	/* Number of global stack ID's */
	int mapped_stack_size;

	/* Define 3 arrays for relationship options */
	long *unmatched;
	long *core;
	long *problem;
	long *direct;
	long *ghost;

	/* Record the ratios */
	double rank_ratio;
	double exclusive_rank;
	double problem_ratio;
	double ghost_ratio;
	double data_to_total_ratio_1;

	/* Array of global consumption */
	long ** consumption;

public:
	/**
	 * Constructor for the consumption map object.
	 * Populates the map relating consumption to stack id.
	 * @param traces A vector of the TraceReader objects - containing the HWM data
	 * @param stacks The CallStackMapper objects - containing the mapping from relevant call stacks
	 * @param rank_ratio The ratio of the rank counts
	 * @param exclusive_rank The exclusive ratio of the rank counts (rank -1)
	 * @param problem_ratio The ratio of the per core problem size
	 * @param ghost_ratio The ratio problem size based on an understanding of the ghost cells from decomposition
	 * @param data_to_total_ratio_1 The ratio of data cells to total cells for trace 1
	 */
	ConsumptionMap(vector<TraceReader *>& traces, CallStackMapper *stacks,
			double rank_ratio, double exclusive_rank, double problem_ratio, double ghost_ratio, double data_to_total_ratio_1);

	/**
	 * Deconstructor for Consumption Map. Frees allocated memory.
	 */
	~ConsumptionMap();


	/**
	 * Get the actual memory consumption 2D array.
	 * @return a 2D array of consumption in bytes.
	 */
	long **getActualConsumption();

	/**
	 * Perform an allocation by allocation size comparison to find a deeper relationship.
	 * @param trace_a The ID of the first core
	 * @param trace_b The ID of the second core
	 * @param globalID The mapped ID of the stack ID
	 */
	void deepStackComparison(int trace_a, int trace_b, int globalID);

	/**
	 * Perform a hazy match between two memory values +- 5%
	 * @param core1 The ID of the first core
	 * @param mem1 The memory from the first core
	 * @param core2 The ID of the second core
	 * @param mem2 The memory from the first core
	 * @param percentage The percentage range to check within, defaults to 5%
	 * @return If a match was found
	 */
	bool hazyMatch(int core1, long mem1, int core2, long mem2,
			double percentage = 5.0);

	/**
	 * Perform a direct match between two memory values - maps to hazy with a percentage of 0
	 * @param core1 The ID of the first core
	 * @param mem1 The memory from the first core
	 * @param core2 The ID of the second core
	 * @param mem2 The memory from the first core
	 * @return If a match was found
	 */
	bool exactMatch(int core1, long mem1, int core2, long mem2);

	/**
	 * A function to print the generated formula to the screen.
	 * @param ranks The number of ranks in the second trace
	 * @param local_problem_size The per processor problem size for the second trace
	 * @param ghost_count1 The number of ghost cells per proc for trace file 1
	 */
	void printFormula(int ranks, double local_problem_size, double ghost_count_1=0.0);

};

#endif /* CONSUMPTIONMAP_H_ */
