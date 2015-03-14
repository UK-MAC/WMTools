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
 * FunctionObj.cpp
 *
 *  Created on: 26 Jul 2012
 *      Author: ofjp
 */

#include "../../include/util/FunctionObj.h"

FunctionObj::FunctionObj(long start_address, long end_address, string name,
		bool elf) {
	this->start_address = start_address;
	this->end_address = end_address;
	this->name = name;
	this->elf = elf;

	this->size = 2 * sizeof(long) + sizeof(int) + name.size() + 1;

}

int FunctionObj::toCharArray(char ** array) {
	const char * n = name.c_str();
	int len = name.size() + 1;
	stringbuf data;
	char *arr = new char[size];
	data.pubsetbuf(arr, size);
	data.sputn((char *) &start_address, sizeof(long));
	data.sputn((char *) &end_address, sizeof(long));
	data.sputn((char *) &len, sizeof(int));
	data.sputn((char *) n, len);

	/* Array gets destroyed by caller */
	*array = arr;
	return size;

}
