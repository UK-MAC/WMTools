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


#ifndef STACKMAP
#define STACKMAP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <deque>
#include <utility>
#include <map>
#include <sstream>

using namespace std;
class StackMap {
private:
	/* Print queue - What is new */
	deque<pair<int, vector<long> > > print_queue;
	long print_queue_size;

	/* Full stack map - everything */
	map<vector<long>, int> call_stack_map;
	map<vector<long>, int>::iterator call_stack_map_it;
	vector<long>::iterator vector_it;
	pair<map<vector<long>, int>::iterator, bool> call_stack_insert;
	int stack_map_ID;

	/** Should we maintain a print queue - Defaults to true*/
	bool print;

	/**
	 * Add a newly formed stack entry into the print queue.
	 * A private function for internal use only.
	 *
	 * @param[in] new_stack The new call stack
	 * @param[in] id The id of the new call stack within the map
	 */
	void addToPrintQueue(vector<long>& new_stack, int id);

public:
	/**
	 * Constructor for StackMap.
	 * Initialises variables.
	 *
	 * @param print Shoule we maintain a print queue - defaults to true.
	 */
	StackMap(bool print = true);

	/**
	 * Add a newly found call stack to the map object.
	 * Before insertion the structure is checked to see if it already contains the object.
	 * If so the existing stack ID is returned, otherwise a new one is generated.
	 *
	 * @param[in] newStack The new call stack to be added to the structure.
	 * @return The ID of the call stack, either old or new.
	 */
	int addStack(vector<long> newStack);

	/**
	 * A means of requesting the new stacks since the last print.
	 *
	 * @param[out] size The size (in bytes) of the array.
	 * @return The array of data for the print queue.
	 */
	char *getPrintQueue(long *size, int *count);
};
#endif
