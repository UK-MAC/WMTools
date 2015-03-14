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


/*
 * ConsumptionMap.cpp
 *
 *  Created on: 3 Aug 2012
 *      Author: ofjp
 */

#include "../../include/util/ConsumptionMap.h"

ConsumptionMap::ConsumptionMap(vector<TraceReader *>& traces,
		CallStackMapper *stacks, double rank_ratio, double exclusive_rank,
		double problem_ratio, double ghost_ratio, double data_to_total_ratio_1) {
	trace_count = traces.size();
	memory_map.resize(trace_count);
	fsa_map.resize(trace_count);
	this->stacks = stacks;
	mapped_stack_size = stacks->getMappedSize();

	/* Store Ratios */
	this->rank_ratio = rank_ratio;
	this->exclusive_rank = exclusive_rank;
	this->problem_ratio = problem_ratio;
	this->ghost_ratio = ghost_ratio;
	this->data_to_total_ratio_1 = data_to_total_ratio_1;

	/* Define list iterators */
	int i, j;

	/* Init relationship arrays - Define 3 arrays for relationship options */
	unmatched = new long[trace_count];
	core = new long[trace_count];
	problem = new long[trace_count];
	direct = new long[trace_count];
	ghost = new long[trace_count];

	/* Init all array to 0 */
	for (i = 0; i < trace_count; i++) {
		unmatched[i] = 0;
		core[i] = 0;
		problem[i] = 0;
		direct[i] = 0;
		ghost[i] = 0;

	}

	/* Decare consumption as null to populate later */
	consumption = NULL;

	/* Loop over each trace */
	for (i = 0; i < trace_count; i++) {

		/* Resize each vector to the size of the number of stacks */
		int stack_size = traces[i]->getCallStacks()->getStackMapSize();
		memory_map[i].resize(mapped_stack_size, 0);
		fsa_map[i].resize(mapped_stack_size);

		/* Fetch the function site allocation lists */
		set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> allocs =
				traces[i]->getFunctionBreakdown();
		set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator>::iterator allocIt =
				allocs.begin();


		/* Iterate over each alloc within the trace and place in the 2D vector */
		while (allocIt != allocs.end()) {
			FunctionSiteAllocation *fsa = (FunctionSiteAllocation *) (*allocIt);
			int map = stacks->mapToGlobal(i, fsa->getStackId());
			memory_map[i][map] += fsa->getMemory();
			fsa_map[i][map] = fsa;
			allocIt++;
		}

	}

}

ConsumptionMap::~ConsumptionMap(){
	/* Free allocated memory */
	delete [] unmatched;
	delete [] core;
	delete [] problem;
	delete [] direct;
	delete [] ghost;

	if(consumption != NULL){
		int i;
		for (i = 0; i < trace_count; i++) {
			delete [] consumption[i];
		}
		delete [] consumption;
	}

}

long **ConsumptionMap::getActualConsumption() {
	/* Declare here, but needs to be freed in deconstructor */
	consumption = new long*[trace_count];

	int i, j;

	/* Loop over traces and call stacks and map from local to global IDs */
	for (i = 0; i < trace_count; i++) {
		consumption[i] = new long[mapped_stack_size];
		for (j = 0; j < stacks->getMappedSize(); j++) {
			consumption[i][j] = memory_map[i][j];
		}
	}

	return consumption;
}

void ConsumptionMap::deepStackComparison(int trace_a, int trace_b, int globalID) {

	/* Look up function site allocations */
	FunctionSiteAllocation *fsa_a = fsa_map[trace_a][globalID];
	FunctionSiteAllocation *fsa_b = fsa_map[trace_b][globalID];

	/* Extract Allocations - Duplicate so as to not destory*/
	vector<long> allocs_a(fsa_a->getAllocSizes());
	vector<long> allocs_b(fsa_b->getAllocSizes());

	/* Check to see if only 1 alloc for each - if so apply hazy match*/
	if (fsa_a->getCount() == 1 && fsa_b->getCount() == 1) {
		bool found = hazyMatch(0, allocs_a[0], 1, allocs_b[0]);
		if (!found) { //Nothing more we can do
			unmatched[trace_a] += allocs_a[0];
			unmatched[trace_b] += allocs_b[0];
		}
		return;
	}

	/* Test to see if the number of allocations is proportional - +- 10% */
	double countRatio = (double) fsa_b->getCount() / fsa_a->getCount();
	if (countRatio > 0.9 && 1.1 >= countRatio) {
		double percentage = abs(1.0 - countRatio);

		/* Hazy match but limited to percentage difference of allocation count */
		bool found = hazyMatch(0, fsa_a->getMemory(), 1, fsa_b->getMemory(),
				percentage);
		if (found)			// Only return if found, otherwise carry on looking
			return;

	}

	/* Attempt to find mapping between elements */
	int i, j;
	for (i = 0; i < allocs_a.size(); i++) {
		bool found = false;
		for (j = 0; j < allocs_b.size(); j++) {

			found = exactMatch(0, allocs_a[i], 1, allocs_b[j]);
			if (found) {
				allocs_b.erase(allocs_b.begin() + j);
				break;
			}

		}
		if (!found) {
			unmatched[trace_a] += allocs_a[i];
		}
	}

	/* Record the unmatched records from trace_b */
	for (j = 0; j < allocs_b.size(); j++) {
		unmatched[trace_b] += allocs_b[j];
	}

	return;

}

bool ConsumptionMap::hazyMatch(int core1, long mem1, int core2, long mem2,
		double percentage) {

	double relationship = (double) mem2 / mem1;
	/* Define upper and lower bounds to be within 5% */
	double r_upper = relationship * (1.0 + (percentage / 100));
	double r_lower = relationship * (1.0 - (percentage / 100));

	/* Check if either is 0 - press no further */
	if (mem1 == 0) {
		unmatched[core2] += mem2;
		return true;
	}
	if (mem2 == 0) {
		unmatched[core1] += mem1;
		return true;
	}

	if (r_upper > 1 && 1 >= r_lower) {
		direct[core2] += mem2;
	} else if (r_upper > rank_ratio && rank_ratio >= r_lower) {
		core[core2] += mem2;
	} else if (r_upper > exclusive_rank && exclusive_rank >= r_lower) {
		core[core2] += mem2;
	} else if (r_upper > problem_ratio && problem_ratio >= r_lower) {
		problem[core2] += mem2;
	} else if (r_upper > ghost_ratio && ghost_ratio >= r_lower) {
		double data_problem = mem2 * data_to_total_ratio_1;
		problem[core2] +=data_problem;
		ghost[core2] += mem2-data_problem;
	} else {
		return false;
	}

	return true;
}

bool ConsumptionMap::exactMatch(int core1, long mem1, int core2, long mem2) {
	return hazyMatch(core1, mem1, core2, mem2, 0.0);
}

void ConsumptionMap::printFormula(int ranks, double lProblem, double ghost_count_1) {
	/* Last attempt - matching unmatched*/
	hazyMatch(0, unmatched[0], 1, unmatched[1]);

	cout << "Unmatched: " << unmatched[0] << " & " << unmatched[1] << "\n";
	cout << "Direct: " << direct[1] << "\n";
	cout << "Core: " << core[1] << "\n";
	cout << "Problem: " << problem[1] << "\n\n\n";

	cout << "For C = Core Count, P = Global Problem.\n Y(C, P)=\n\t"
			<< direct[1] << " + " << core[1] / ranks << "*C + "
			<< (problem[1]) / lProblem << "*(P/C)";
	if(ghost_count_1 > 0.0 && ghost[1] > 0.0)
			cout << " + " << ghost[1]/ghost_count_1 << " Ghost cells";
	cout << "\n";

}

