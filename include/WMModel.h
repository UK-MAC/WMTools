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
 * WMModel.h
 *
 *  Created on: 31 Jul 2012
 *      Author: ofjp
 */

#ifndef WMMODEL_H_
#define WMMODEL_H_

#include "util/TraceReader.h"
#include "util/CallStackMapper.h"
#include "util/ConsumptionMap.h"

#include <iostream>
#include <vector>

using namespace std;

/**
 * WMModel is currently a work in progress.
 *
 * The idea is to replay multiple trace files at the point of high water mark.
 * We then perform analysis on the similarities and differences between the allocation sizes and locations.
 *
 * The idea is to automatically extract trends in allocation size as the core count / problem size changes.
 *
 * From this we generate a model, which can be used to predict consumption at larger scale.
 */
class WMModel {
private:
	int trace_count;

	/* Vectors of the data across different trace files */
	vector<string> file_names;
	vector<long*> sizes;
	vector<long> global_sizes;
	vector<int*> decomps;
	vector<int> ranks;
	vector<TraceReader *> readers;
	vector<TraceReader *> second_readers;
	vector<long> hwm_memory;
	vector<long> hwm_times;

	/* Data storage objects */
	CallStackMapper *stack_maps;
	ConsumptionMap *memory_map;

public:
	/**
	 * Constructor for the WMModel object
	 * @param filenames A list of the filenames of the trace files to model from
	 * @param sizes A list of the global problem sizes corresponding to the trace files assuming 3 dimensions
	 * @param decomps A list of the processor decompositions corresponding to the trace files assuming 3 dimensions
	 */
	WMModel(vector<string>& filenames, vector<long *>& sizes, vector<int *>& decomps);

	/**
	*  De-constructor for the WMModel object, to free any memory allocated by it.
	*/
	~WMModel();

	/**
	*  Print the equation generated by the model to standard out.
	*/
	void printFormula();	
};


#endif /* WMMODEL_H_ */
