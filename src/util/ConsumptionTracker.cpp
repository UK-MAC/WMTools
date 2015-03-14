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


#include "../../include/util/ConsumptionTracker.h"

ConsumptionHWMTracker::ConsumptionHWMTracker(string filename,
		bool graph, bool samples) {

	/* Init variables to 0 */
	hwm = 0;
	hwm_time = 0.0;
	hwmID = 0;

	curr_memory = 0;
	curr_time = 0.0;
	currID = 0;

	/* Store passed parameters */
	this->graph = graph;
	this->samples = samples;
	consumption = new ConsumptionGraph(filename, samples);

}

ConsumptionHWMTracker::~ConsumptionHWMTracker() {
	delete consumption;
	allocation_map.clear();
}

long ConsumptionHWMTracker::addAllocation(MallocObj& malloc) {
	currID++;
	curr_memory += malloc.getSize();
	curr_time += malloc.getTime();

	allocation_map.insert(pair<long, MallocObj>(malloc.getPointer(), malloc));

	/* If we are graphing then add point to consumption graph */
	if (graph)
		consumption->addAllocation(curr_time, curr_memory);

	return currID;
}

long ConsumptionHWMTracker::addFree(FreeObj& free) {

	/* Check if we are at HWM */
	checkHWM();

	/* Update variables */
	currID++;
	curr_time += free.getTime();

	/* Try to fetch corresponding malloc, to know free size */
	MallocObj * malloc = getAllocation(free.getPointer());
	if (malloc != NULL) {

		/* Decrease mem by corresponding values */
		curr_memory -= malloc->getSize();
		allocation_map.erase(free.getPointer());

		/* If we are graphing then add point to consumption graph */
		if (graph)
			consumption->addAllocation(curr_time, curr_memory);
	}

	return currID;
}

MallocObj *ConsumptionHWMTracker::getAllocation(long pointer) {
	MallocObj * res;
	allocation_map_it = allocation_map.find(pointer);
	if (allocation_map_it == allocation_map.end())
		return NULL;

	return &allocation_map_it->second;
}

void ConsumptionHWMTracker::finish() {
	/* Check if we are at HWM */
	checkHWM();

	consumption->setLocalHwm(hwm);

	if (graph && !samples)
		consumption->dumpGraphToFile(curr_time);
}

void ConsumptionHWMTracker::checkHWM(){
	if (curr_memory > hwm) {
			hwm = curr_memory;
			hwm_time = curr_time;
			hwmID = currID;
	}
}


set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> ConsumptionHWMTracker::getFunctionBreakdown() {
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> functions;

	allocation_map_it = allocation_map.begin();
	long total_count = 0;
	while (allocation_map_it != allocation_map.end()) {
		/* Make a new FunctionSiteAllocation object from the values from the allocation map */
		FunctionSiteAllocation * fsa = new FunctionSiteAllocation(
				allocation_map_it->second.getStackID(),
				allocation_map_it->second.getSize());
		total_count += allocation_map_it->second.getSize();
		/* Try to inser the new object - but test to see if it clashed with an existing stack id value. */
		pair<
				set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator>::iterator,
				bool> insert_test = functions.insert(fsa);

		if (insert_test.second == false) {
			FunctionSiteAllocation * fsa_2 = *insert_test.first;
			fsa_2->addMemory(allocation_map_it->second.getSize());
		}

		allocation_map_it++;
	}


	return functions;

}
