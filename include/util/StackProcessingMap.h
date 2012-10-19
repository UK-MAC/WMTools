/*
 * StackProcessingMap.h
 *
 *  Created on: 27 Jul 2012
 *      Author: ofjp
 */

#ifndef STACKPROCESSINGMAP_H_
#define STACKPROCESSINGMAP_H_

#include <map>
#include <vector>

using namespace std;

/**
 * StackProcessingMap is an object class to store and manage call stacks.
 *
 * The class maps each call stack to a unique ID and stores them in a map for quick reference.
 */
class StackProcessingMap {
private:
	map<int, vector<long> > call_stacks;
	map<int, vector<long> >::iterator call_stacks_it;

public:
	StackProcessingMap();

	/**
	 * Add a call stack entry to the map, read from a trace file, so a known ID.
	 *
	 * @param id The ID of the call stack.
	 * @param stack The vector of addresses for the call stack
	 */
	void addCallStack(int id, vector<long>& stack);

	/**
	 * Add a call stack entry to the map, read from a trace file, so a known ID.
	 * For this method providing only the bare array.
	 *
	 * @param id The ID of the call stack.
	 * @param size The number of elements in the vector.
	 * @param data_array The data array representing the vector.
	 */
	void addCallStack(int id, int size, long * data_array);

	/**
	 * Return the vector of call stacks for the given ID.
	 * @param id The ID to fetch the vector for.
	 * @return The vector, or null if not found.
	 */
	vector<long> getVector(int id);

	/**
	 * A somewhat dangerous function to request all the saved call stacks.
	 * So they can be used at a later date
	 * @return
	 */
	map<int, vector<long> > getStacks() {
		return call_stacks;
	}

	/**
	 * Function to return the number of call stacks contained within the object.
	 *
	 * @return The number of call stacks contained within this trace.
	 */
	int getStackMapSize() {
		return call_stacks.size();
	}
};

#endif /* STACKPROCESSINGMAP_H_ */
