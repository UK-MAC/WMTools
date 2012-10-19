#ifndef VIRTUALMEMORYDATA
#define VIRTUALMEMORYDATA

#include <deque>
#include <link.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include "FunctionObj.h"
#include <sstream>

using namespace std;

/**
 * VirtualMemoryData is an object class to represent the functions address space of static library functions.
 * It is a simple struct style class to wrap the data.
 */
class VirtualMemoryData {
private:
	deque<FunctionObj *> functions;

	long array_size;

public:
	VirtualMemoryData();

	/**
	 * Function to add function data to the store.
	 * Also update the running total of data stored, for future use.
	 *
	 * @param start The start address of the function.
	 * @param end The end address of the function.
	 */
	void addData(long start, long end, string name);

	/**
	 * A function to request the function data (start address, end address and name) from the store in address order.
	 *
	 * @param[out] array The array to copy to data to.
	 * @param[out] length The size (B) of the new array.
	 * @return The number of functions recorded in the array.
	 */
	int getData(char ** array, int *length);

};

#endif
