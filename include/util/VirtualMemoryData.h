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
