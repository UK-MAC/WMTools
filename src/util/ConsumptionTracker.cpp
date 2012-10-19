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
		consumption->dumpGraphToFile();
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

	while (allocation_map_it != allocation_map.end()) {
		//Make a new FunctionSiteAllocation object from the values from the allocation map
		FunctionSiteAllocation * fsa = new FunctionSiteAllocation(
				allocation_map_it->second.getStackID(),
				allocation_map_it->second.getSize());

		//Try to inser the new object - but test to see if it clashed with an existing value.
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
