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
 * StackProcessingMap.cpp
 *
 *  Created on: 27 Jul 2012
 *      Author: ofjp
 */

#include "../../include/util/StackProcessingMap.h"

StackProcessingMap::StackProcessingMap() {

}

void StackProcessingMap::addCallStack(int id, vector<long>& stack) {
	call_stacks.insert(pair<int, vector<long> >(id, stack));
}

void StackProcessingMap::addCallStack(int id, int size, long * data_array) {
	vector<long> new_vec;
	int i;

	for (i = 0; i < size; i++) {
		new_vec.push_back(data_array[i]);
	}
	addCallStack(id, new_vec);
}

vector<long> StackProcessingMap::getVector(int id) {
	call_stacks_it = call_stacks.find(id);
	if (call_stacks_it == call_stacks.end()) {
		vector<long> tmp;
		return tmp;
	}
	return call_stacks_it->second;
}
