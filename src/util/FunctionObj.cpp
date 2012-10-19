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
