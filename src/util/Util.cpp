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


#include "../../include/util/Util.h"

string WMUtils::cppDemangle(string input) {

	int status;

	/* Call external function to try to demangle */
	char * real_str = abi::__cxa_demangle(input.c_str(), 0, 0, &status);

	/* Only store the result if the correct status is returned */
	if (status == 0) {
		string str(real_str);
		delete [] real_str;
		return str;
	} else {
		string str(input);
		delete [] real_str;
		return str;
	}
}

vector<long> WMUtils::stackToVector(stack<long> input) {
	/* Make the vector to store the stack in */
	vector<long> my_vec;

	/* Loop over the stack populating the vector */
	while (!input.empty()) {
		my_vec.push_back(input.top());
		input.pop();
	}

	return my_vec;
}

string WMUtils::makeFileName(bool makeNew) {
	/* Retrieve the rank */
	int rank = getMPIRank();

	/* If rank 0 make a folder name based on root */
	string folder(WMTRACEDIR);
	if (rank == 0) {
		if (makeNew)
			folder.assign(makeUniqueFolder(folder));
		else
			folder.assign(findUniqueFolder(folder));
	}

	/* Broadcast the folder name  to all ranks */
	char folder_str[100];
	sprintf(folder_str, "%s", folder.c_str());

	/* Only perform the broadcast if we are using MPI */
#ifndef NO_MPI
	MPI_Bcast(folder_str, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

	return stichFileName(folder_str, rank);
}

string WMUtils::stichFileName(string base, int rank) {
	/* Group rank to folder name to make a filename */
	stringstream file;
	file << base << "/trace-" << rank << ".z";

	return file.str();
}

#ifndef NO_MPI
int WMUtils::getMPIRank() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}
#else
int WMUtils::getMPIRank() {
	/* Implemented without MPI use, returns 0 */
	return 0;
}
#endif

#ifndef NO_MPI
int WMUtils::getMPICommSize() {
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	return size;
}
#else
int WMUtils::getMPICommSize() {
	/* Implemented without MPI use, returns 1 */
	return 1;
}
#endif

#ifndef NO_MPI
string WMUtils::getMPIProcName() {
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;

	/* Fetch the processor name */
	MPI_Get_processor_name(processor_name, &name_len);
	string name(processor_name);
	return name;
}
#else
string WMUtils::getMPIProcName() {
	/* Implemented without MPI use, returns "Unknown" */
	string name("Unknown");
	return name;
}
#endif

string WMUtils::stripSuffix(string filename) {
	size_t pos = filename.find_last_of('.');
	return filename.substr(0, pos);
}

string WMUtils::makeGraphFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISGRAPH);
	return prefix;
}

string WMUtils::makeFunctionsFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISFUNCTIONS);
	return prefix;
}

string WMUtils::makeAllocationsFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISALLOCATIONS);
	return prefix;
}

string WMUtils::extractFolder(string filename) {
	size_t pos = filename.find_last_of('/');
	return filename.substr(0, pos);
}

double WMUtils::calculateStandardDeviation(long * array, int size) {

	/* Protect against divide by 0 error */
	if (size == 1)
		return 0;

	double mean = 0.0;
	double sosd = 0.0;

	/* Calculate Mean*/
	int i;
	for (i = 0; i < size; i++)
		mean += (double) array[i];

	mean /= (double) size;

	/* Calculate sum of square deviations */
	for (i = 0; i < size; i++)
		sosd += pow((double) array[i] - mean, 2);

	/* Divide through by one less than size */
	sosd /= (double) (size - 1);

	/* Calculate & return square root */
	return sqrt(sosd);
}

string WMUtils::makeUniqueFolder(string base) {
	int test = 0;
	struct stat myStat;
	string folder(base);
	/* Check to see if the folder name on its own exists, if not make it and store it. */
	if (stat(base.c_str(), &myStat) != 0) {
		mkdir(base.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		folder.assign(base);
		return folder;
	}

	/* Folder exists so pepend an integer to make a unique file dir */
	char s[base.size() + 5];
	do {
		sprintf(s, "%s%04d", base.c_str(), test);
		test++;
	} while (stat(s, &myStat) == 0);

	/* Make the directory and save it's name */
	mkdir(s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	folder.assign(s);
	return folder;
}

string WMUtils::findUniqueFolder(string base) {
	int test = 0;
	struct stat my_stat;
	string folder(base);

	/* Folder exists so pepend an integer to make a unique file dir */
	char s[base.size() + 5];
	sprintf(s, "%s", base.c_str());
	do {
		folder.assign(s);
		sprintf(s, "%s%04d", base.c_str(), test);
		test++;
	} while (stat(s, &my_stat) == 0);

	return folder;
}

int WMUtils::countRunSize(string base) {
	int count = -1;
	struct stat my_stat;
	char name[200];
	do {
		count++;
		sprintf(name, "%s/trace-%d.z", base.c_str(), count);
	} while (stat(name, &my_stat) == 0);

	return count;
}
