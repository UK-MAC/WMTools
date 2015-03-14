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


#ifndef FREE_OBJ
#define FREE_OBJ

#include <stdio.h>

/**
 * The FreeObj class is a glorified struct to store a free event.
 *
 * The primary purpose is to store the pointer and time of the free event.
 *
 * Similar to malloc_obj
 */
class FreeObj {
private:
	long alloc_pointer;
	float alloc_time;

public:
	/**
	 * Constructor for the free object
	 * @param pointer The address of the memory to be freed
	 * @param time The time delta since the last event
	 */
	FreeObj(long pointer, float time) {
		alloc_pointer = pointer;
		alloc_time = time;
	}

	/**
	 * Getter for the free address
	 * @return The free pointer
	 */
	long getPointer() {
		return alloc_pointer;
	}

	/**
	 * Getter for the free time
	 * @return The free time
	 */
	float getTime() {
		return alloc_time;
	}
};
#endif

