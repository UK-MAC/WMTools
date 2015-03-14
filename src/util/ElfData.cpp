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


#include "../../include/util/ElfData.h"


ElfData::ElfData() {
	assert(elf_version(EV_CURRENT) != EV_NONE);

	assert((fd = open(__progname_full, O_RDONLY, 0)) >= 0);

	assert((e = elf_begin(fd, ELF_C_READ, NULL)) != NULL);

	assert(elf_kind(e) == ELF_K_ELF);

}


long ElfData::getElfMem() {
	long totalMem = 0;

	int i;
	size_t n;
	/* Causes problems on intel C++ */
	//assert(elf_getphnum(e, &n) == 1);
	GElf_Ehdr elfhdr;
	assert(gelf_getehdr(e, &elfhdr) != 0);
	n = (size_t) elfhdr.e_phnum;

	for (i = 0; i < n; i++) {
		if (gelf_getphdr(e, i, &phdr) != &phdr)
			return totalMem;

		totalMem += (long) phdr.p_memsz;

	}
	return totalMem;

}


int ElfData::getFunctions(char ** data, long * size) {

	/* Construct temporary storage for our functions */
	deque < pair<long, string> > functions;
	long running_size = 0;
	int function_count = 0;

	int count = 0;
	int ii;
	int found = 0;


	Elf_Scn *scn = NULL;

	/* Loop over the file symbol headers looking for functions frame*/
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		gelf_getshdr(scn, &shdr);
		if (shdr.sh_type == SHT_SYMTAB) {
			// found a symbol table, go print it.
			found = 1;
			break;
		}
	}

	/* Ensure that we have found it */
	assert(found == 1 && shdr.sh_entsize > 0);

	/* Calculate the count from the size */
	Elf_Data *elfdata = elf_getdata(scn, NULL);
	count = shdr.sh_size / shdr.sh_entsize;

	/* Loop through function list, adding the relevant ones to our list */
	for (ii = 0; ii < count; ++ii) {
		GElf_Sym sym;
		gelf_getsym(elfdata, ii, &sym);

		string function_name = elf_strptr(e, shdr.sh_link, sym.st_name);

		long function_address = sym.st_value;

		/* Only consider functions with a non 0 address + with a name */
		if (function_address != 0 && function_name.length() > 0) {
			/* Add the function to our list */
			functions.push_back(
					pair<long, string>(function_address, function_name));
			running_size += sizeof(long) + sizeof(int)
					+ (function_name.size() + 1);

		}

	}

	/* Order the deque Structure */
	DequeSorter ds;
	sort(functions.begin(), functions.end(), ds);

	/* Size the array by the count of elements*/
	stringbuf out_data;
	char *arr = new char[running_size];
	out_data.pubsetbuf(arr, running_size);

	long currentWritten = 0;
	/* Loop over functions outputting them to the array */
	while (!functions.empty()) {
		function_count++;
		out_data.sputn((char *) &(functions.front().first), sizeof(long));

		int size = (int) functions.front().second.length();
		size++;

		out_data.sputn((char *) &size, sizeof(int));

		const char * str = functions.front().second.c_str();

		out_data.sputn((char *) str, sizeof(char) * size);

		functions.pop_front();

	}
	*data = arr;
	*size = running_size;

	/* Return the function count, to know how many elements in the array */
	return function_count;

}
