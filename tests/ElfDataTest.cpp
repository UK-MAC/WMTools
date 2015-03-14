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


#include "../include/util/ElfData.h"
#include "../include/util/util.h"

#include <iostream>
#include <vector>
#include <assert.h>

using namespace std;

int main(){
	ElfData * elf = new ElfData();

	long staticmem = elf->getElfMem();

	void *array;
	long arraySize;
	int count = elf->getFunctions(&array, &arraySize);

	cout << "Static Memory: " << staticmem << " (B)\n\nFunctions(" << arraySize <<") " << count << ":\n";

	void * tmpArr = array;
	int i;
	for(i = 0; i < count; i++){
	//while(true){
		long addr;
		memcpy(&addr, tmpArr, sizeof(long));
		cout << "Address: "<< addr;
		tmpArr=(void *)tmpArr+sizeof(long);
		int size;
		memcpy(&size, tmpArr, sizeof(int));
		tmpArr=(void *)tmpArr+sizeof(int);
		cout << " Size: "<< size;
		char *str = new char[size];
		memcpy(str, tmpArr, sizeof(char)*size);
		tmpArr=(void *)tmpArr+sizeof(char)*size;
		cout << " Name: " << str << " ";
		string demag = cppDemangle(string(str));
		cout << "Demangled: " << demag << "\n";

	}
/*
	int read = 0;
	while(read <= arraySize){
		long addr;
		memcpy(&addr, (void*)array+read, sizeof(long));
		read+=sizeof(long);
		cout << "Address: "<< addr;

		int size;
		memcpy(&size, (void*)array+read, sizeof(int));
		read+=sizeof(int);

		cout << " Size: " << size;


		//char *str = new char[size];
		//memcpy(str, (void*)array+read, size*sizeof(char));
		//read+=size*sizeof(char);

		//cout << " Name: " << str << "\n";

	}
*/
/*
	int i;
	for(i=0;i<arraySize;i++){
		cout << ((char *)array)[i];
	}
	cout << "\n";
*/
}
