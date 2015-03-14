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
 * FunctionObj.h
 *
 *  Created on: 25 Jul 2012
 *      Author: ofjp
 */

#ifndef FUNCTIONOBJ_H_
#define FUNCTIONOBJ_H_

#include <iostream>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <sstream>

using namespace std;

/**
 * FunctionObj is an object designed to store a function, represented by a start and end memory address and a name.
 * Specifically this class is designed for virtual libraries, whose addresses are not continuous.
 */
class FunctionObj {
private:
	long start_address;
	long end_address;
	string name;
	/** The size (B) that this object will represent when copied to a void array. */
	int size;

	/* Is this an elf Function */
	bool elf;

public:
	/**
	 * Constructor for the FunctionObj class.
	 *
	 * @param start_address The start address of the function / library.
	 * @param end_address The end address of the function / library.
	 * @param name The name of the function / library.
	 */
	FunctionObj(long start_address, long end_address, string name, bool elf);

	/**
	 * Function to get the end address of the function object.
	 *
	 * @return The end address of the function.
	 */
	long getEndAddress() const {
		return end_address;
	}

	/**
	 * Function to get the function name.
	 *
	 * @return The function name.
	 */
	const string& getName() const {
		return name;
	}

	/**
	 * Function to get the start address of the function object.
	 *
	 * @return The start address of the function.
	 */
	long getStartAddress() const {
		return start_address;
	}

	/**
	 * Function to convert the object to a char array.
	 * Copies the data in the form of:
	 * <(long)Start address><(long)End Address><(int)Name size><Name>
	 *
	 * @param[out] array The array to copy the data into.
	 * @return The size of the array produced.
	 */
	int toCharArray(char ** array);

	/**
	 * Get the array size of this object.
	 *
	 * @return The size (B) that this object represents.
	 */
	int getSize() {
		return size;
	}

	/**
	 * Function to check if a function address is between a given range.
	 *
	 * @param address The address of the function to check.
	 * @return If the address sits between this functions range.
	 */
	bool withinRange(long address) {
		return (address >= start_address && address < end_address);
	}

	/**
	 * Query if this function is obtained from the elf descriptor.
	 * This represents it as a static library function, e.g. belonging to the source binary.
	 *
	 * @return If this function was obtained from the elf descriptor.
	 */
	bool isElf() {
		return elf;
	}

	struct comparator {
		bool operator ()(FunctionObj *a, FunctionObj *b) {
			//cout << "Function " << a->getName() << " ( " << a->getStartAddress() << " -> " << a->getEndAddress() << ") Vs. " << b->getName() << " ( " << b->getStartAddress() << " -> " << b->getEndAddress() << ")" ;
			return a->getStartAddress() < b->getStartAddress()
					&& !a->withinRange(b->getStartAddress());

		}
	};

};

#endif /* FUNCTIONOBJ_H_ */

