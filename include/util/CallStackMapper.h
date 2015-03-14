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
 * CallStackMapper.h
 *
 *  Created on: 31 Jul 2012
 *      Author: ofjp
 */

#ifndef CALLSTACKMAPPER_H_
#define CALLSTACKMAPPER_H_

#include <vector>
#include <map>
#include <set>

#include "FunctionObj.h"
#include "TraceReader.h"
#include "StackProcessingMap.h"

using namespace std;

/**
 * A class to construct a mapping between a number of trace files.
 * Takes as input the function maps, and call stacks and builds a mapping between them.
 * We maintain a 2D stack of call stack IDs, indexed first by trace, then by stackID.
 */
class CallStackMapper {
private:
	/* Record of the input trace readers */
	vector<TraceReader *> traces;

	vector<StackProcessingMap *> call_stacks;
	vector<FunctionMap *> functions;

	/* Record the call stacks as mappings with the mapped ID */
	vector<vector<long> > mapped;

	vector<vector<int> > map;

	int trace_count;

	int stackID_count;

	/**
	 * A function to compare two call stack objects.
	 * As we have the vectors accessible we don't need to pass the data structure.
	 * We must check to see if the addresses directly match.
	 * If not check to see if they are dynamic library addresses.
	 * In which case check the strings of the library.
	 *
	 * @param trace_a The ID of the first trace, to reference the vector from.
	 * @param stackID_a The ID of the first call stack to check.
	 * @param trace_b The ID of the second trace, to reference the vector from.
	 * @param stackID_b The ID of the call stack to check against.
	 *
	 * @return If the two are considered equal.
	 */
	bool compareCallStacks(int trace_a, int stackID_a, int trace_b, int stackID_b);

public:
	/**
	 * Constructor for the CallStackMapper object.
	 * @param traces A vector of the traces (in TraceReader form) to map from
	 */
	CallStackMapper(vector<TraceReader *>& traces);

	/**
	 * Return the mapped index for the call stack of the specific trace.
	 * @param trace The ID of the trace to check for.
	 * @param stackID The ID of the call stack to trace back to.
	 *
	 * @return The mapped call stack ID.
	 */
	int mapToGlobal(int trace, int stackID);

	/**
	 * A function to map from a call stack on one trace to a call stack of another trace.
	 * Returns -1 for no mapping.
	 *
	 * @param trace_a The index of the first trace.
	 * @param stackID The ID of the call stack to query for.
	 * @param trace_b The index of the trace to map to.
	 *
	 * @return The mapped stack ID from trace A to trace B.
	 */
	int lookUpMapping(int trace_a, int stackID, int trace_b);

	/**
	 * Find the size of the mapped array.
	 * @return The size of the mapped call stack vector.
	 */
	int getMappedSize() {
		return mapped.size();
	}
};

#endif /* CALLSTACKMAPPER_H_ */
