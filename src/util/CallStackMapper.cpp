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
 * CallStackMapper.cpp
 *
 *  Created on: 31 Jul 2012
 *      Author: ofjp
 */

#include "../../include/util/CallStackMapper.h"

CallStackMapper::CallStackMapper(vector<TraceReader *>& traces) {

	this->traces = traces;
	stackID_count = 0;
	trace_count = traces.size();

	//Resize map to the number of traces.
	map.resize(trace_count);
	call_stacks.resize(trace_count);
	functions.resize(trace_count);

	/* Push an empty vector into the mapped to represent the unfound element later - prevents -1 being used as index*/
	vector<long> tmp;
	mapped.push_back(tmp);

	/* Start proper count from 1 */
	stackID_count++;

	/* Calculate the max number of call stacks if no overlap = sum of all callstacks */
	int max_stack_count = 0;

	int i;
	for (i = 0; i < trace_count; i++) {
		call_stacks[i] = traces[i]->getCallStacks();
		functions[i] = traces[i]->getFunctionMap();

		max_stack_count += call_stacks[i]->getStackMapSize();
	}

	//Not sure if we need to resize the map to the max capacity?
	//mapped.resize(maxStackCount);

	int j, k, l;

	//First pass through to init everything to 0 signifying no match
	for (j = 0; j < trace_count; j++) {
		map[j].resize(call_stacks[j]->getStackMapSize(), 0);
	}

	//Loop over traces
	for (j = 0; j < trace_count; j++) {

		//For each call stack
		for (i = 0; i < call_stacks[j]->getStackMapSize(); i++) {
			int mapID = map[j][i];
			//Check if we have already mapped to this element
			if (mapID == 0) {
				mapID = stackID_count;
				map[j][i] = mapID;
				mapped.push_back(call_stacks[j]->getVector(i));
				stackID_count++;
			} else {
				continue;
			}

			//Consider each other trace later than this one.
			for (k = (j + 1); k < trace_count; k++) {
				bool found = false;
				//Consider every call stack of this new trace
				for (l = 0; !found && l < call_stacks[k]->getStackMapSize();
						l++) {
					if (map[k][l] == 0) { //-1 signifies no match, so only process these
						if (compareCallStacks(j, i, k, l)) {
							map[k][l] = mapID;
							found = true;
						}

					}
				}

			}
		}

	}

}

bool CallStackMapper::compareCallStacks(int trace_a, int stackID_a, int trace_b,
		int stackID_b) {
	bool result = false;
	vector<long> stack_a = call_stacks[trace_a]->getVector(stackID_a);
	vector<long> stack_b = call_stacks[trace_b]->getVector(stackID_b);

	/* Resolve function addresses to names */
	vector < string > stack_a_strings = traces[trace_a]->getCallStack(stackID_a);
	vector < string > stack_b_strings = traces[trace_b]->getCallStack(stackID_b);

	//Quick check on size to eliminate obvious mismatches
	if (stack_a.size() != stack_b.size()) {
		//cout << "False: Different Sizes: " << stacka.size() << " != " << stackb.size() << "\n";
		return false;
	}

	//Store the size, as they are both the same
	int size = stack_a.size();

	int i;

	//Loop over the elements and check if they are the same
	for (i = 0; i < size; i++) {

		/* For now just resolve the function names and use them */
		if (stack_a_strings[i].compare(stack_b_strings[i]) != 0)
			return false;

	}

	return true;
}

int CallStackMapper::mapToGlobal(int trace, int stackID) {
	//Error check to ensure bounds.
	if (trace < 0 || trace >= map.size() || stackID < 0
			|| stackID >= stackID_count) {
		return 0;
	}

	return map[trace][stackID];

}

int CallStackMapper::lookUpMapping(int trace_a, int stackID, int trace_b) {
	//Get the mapped ID for trace a.
	int mapID = map[trace_a][stackID];

	//Search the Map for the MapID
	int i;
	for (i = 0; i < call_stacks[i]->getStackMapSize(); i++) {
		if (map[trace_b][i] == mapID)
			return i;
	}
	return -1;
}
