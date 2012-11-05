/*
 * FunctionSiteAllocation.h
 *
 *  Created on: 30 Jul 2012
 *      Author: ofjp
 */

#ifndef FUNCTIONSITEALLOCATION_H_
#define FUNCTIONSITEALLOCATION_H_

#include <deque>

using namespace std;

/**
 * The FunctionSiteAllocation object class maintains information about the memory allocated to the call stack.
 *
 * It also records the number of allocations + a vector of the individual sizes.
 * This is used later in the model to provide better comparisons.
 */
class FunctionSiteAllocation {
private:
	/* The ID of the call stack associated with this allocation collection*/
	int stackID;
	/* The memory(B) allocated by this call stack */
	long memory;
	/* The number for allocations attributed to this call stack */
	int count;
	/* Individual allocation memory consumptions */
	vector<long> allocs;

public:
	/**
	 * Constructor for the FunctionSiteAllocation object.
	 * Essentially a struct to wrap three items of data.
	 *
	 * @param stackID The ID of the call stack represented by this object.
	 * @param memory The first install of memory
	 */
	FunctionSiteAllocation(int stackID, long memory) {
		this->memory = 0;
		this->count = 0;
		this->stackID = stackID;
		addMemory(memory);
	}

	/**
	 * A function to add another allocation to this object.
	 * Adds the memory of the allocation, and increments the counter.
	 *
	 * @param memory The new memory to add the the existing memory of the object.
	 */
	void addMemory(long memory) {
		this->memory += memory;
		this->count++;
		allocs.push_back(memory);
	}

	/**
	 * Getter for number of allocations contained within this object.
	 *
	 * @return The allocation count for this stack ID.
	 */
	int getCount() const {
		return count;
	}

	/**
	 * Getter for the amount of memory contained within this object in bytes.
	 *
	 * @return The amount of memory associated with this  stack ID.
	 */
	long getMemory() const {
		return memory;
	}

	/**
	 * Getter for the stack ID of this object.
	 *
	 * @return The stack ID of the call stack this object represents.
	 */
	int getStackId() const {
		return stackID;
	}

	/**
	 * Fetch a deque of the allocation sizes.
	 * @return The allocation sizes.
	 */
	vector<long> getAllocSizes() {
		return allocs;
	}

	/**
	 * A custom comparitor for the FunctionSiteAllocation object based on the Stack ID of the call stack each object represents.
	 */
	struct comparator {
		bool operator ()(FunctionSiteAllocation *a, FunctionSiteAllocation *b) {
			return a->getStackId() < b->getStackId();
		}
	};
	/**
	 * A custom comparitor for the FunctionSiteAllocation object based on the volume of memory for the call stack each object represents.
	 */
	struct comparatorMem {
		bool operator ()(FunctionSiteAllocation *a, FunctionSiteAllocation *b) {
			if(a->getMemory() == b->getMemory())
				return a->getStackId() < b->getStackId();
			else
				return a->getMemory() < b->getMemory();
		}
	};
};

#endif /* FUNCTIONSITEALLOCATION_H_ */
