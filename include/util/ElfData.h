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


#ifndef ELFDATA
#define ELFDATA

#include "Util.h"

#include <iostream>
#include <algorithm>
#include <err.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <sysexits.h>
#include <unistd.h>
#include <string.h>
#include <set>
#include <string>
#include <utility>
#include <deque>
#include <sstream>

/* Used to extract the path to binary */
extern const char *__progname_full;

using namespace std;

/**
 * The ElfData object is used to read the elf headers from the currently executing binary.
 *
 * The object reads for two specific bits of information, the static memory allocations and library function addresses.
 */
class ElfData {

private:
	/* File descriptor objects */
	int fd;
	Elf *e;
	char *id, bytes[5];

	/* Global elf objects */
	GElf_Phdr phdr;
	GElf_Shdr shdr;

public:
	/**
	 * Constructor for the ElfData object.
	 * Initialises the read.
	 */
	ElfData();

	/**
	 * A function to extract the static memory size out of the current binary through the Elf headers.
	 *
	 * @return The size of the static memory.
	 */
	long getElfMem();

	/**
	 * A function to extract the function names of the current binary from the Elf headers.
	 * Stored in the format:
	 * 		<(long)Start address><(int)String size><Function name>
	 * 		<(long)Start address><(int)String size><Function name>
	 * 		.....
	 *
	 *
	 * First extracts the functions into a deque then maps that into a void *
	 *
	 * @param[out] data The void * array to write the data into.
	 * @param[out] size The size of the new array.
	 * @return The number of entries.
	 */
	int getFunctions(char ** data, long * size);

	/**
	 * A structure used to sort the function addresses by address space.
	 */
	struct DequeSorter {
		bool operator ()(const pair<long, string> &a,
				const pair<long, string> &b) {
			return a.first < b.first;
		}
	};
};

#endif
