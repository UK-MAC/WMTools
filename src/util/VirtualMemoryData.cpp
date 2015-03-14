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


#include "../../include/util/VirtualMemoryData.h"
#include <stdio.h>

int callback(struct dl_phdr_info *info, size_t size, void *data) {
	VirtualMemoryData * vmd = (VirtualMemoryData *) data;
	int j;


	long min, max;

	/* Fetch the header information for loaded libraries - Determine the start and end of the address space */
	for (j = 0; j < info->dlpi_phnum; j++) {
		long start = (long) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
		long end = (long) (info->dlpi_addr + info->dlpi_phdr[j].p_vaddr
				+ info->dlpi_phdr[j].p_memsz);
		if (j == 0) {
			min = start;
			max = end;
		} else {
			if (start < min)
				min = start;
			if (end > max)
				max = end;
		}

	}

	/* Make a new string with the lib name - Then add it to the data store */
	string lib(info->dlpi_name);
	if (lib.length() > 0) {
		vmd->addData(min, max, lib);
	}

	return 0;
}

VirtualMemoryData::VirtualMemoryData() {
	array_size = 0;

	dl_iterate_phdr(callback, this);

}

void VirtualMemoryData::addData(long start, long end, string name) {
	FunctionObj *fo = new FunctionObj(start, end, name, false);
	functions.push_back(fo);

	/* Store the change in size to help with the output later */
	array_size += fo->getSize();
}

int VirtualMemoryData::getData(char ** array, int *length) {
	FunctionObj::comparator s;

	/* Order the functions - by address space */
	sort(functions.begin(), functions.end(), s);

	int function_count = functions.size();

	/* Set up string buffer */
	char *data = new char[array_size];
	stringbuf out_data;
	out_data.pubsetbuf(data, array_size);

	/* Loop over the functions, converting and destroying the objects */
	while (!functions.empty()) {
		FunctionObj *fo = functions.front();
		functions.pop_front();

		char * fun_data;
		int fun_size = fo->toCharArray(&fun_data);
		out_data.sputn((char *) fun_data, fun_size);

		delete[] fun_data;
		delete fo;

	}

	*array = data;
	*length = array_size;

	return function_count;

}
