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
