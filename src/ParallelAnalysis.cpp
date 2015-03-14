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
 * ParallelAnalysis.cpp
 *
 *  Created on: 1 Oct 2012
 *      Author: ofjp
 */

#include "../include/WMAnalysis.h"

int main(int argc, char* argv[]) {

	MPI_Init(&argc, &argv);

	bool graph = false;
	bool functions = false;
	bool allocations = false;

	bool singleFile = false;

	string filename = WMUtils::makeFileName();

	int rank = WMUtils::getMPIRank();
	int comm = WMUtils::getMPICommSize();

	for (int i = 1; i < argc; i++) {
		string arg(argv[i]);

		if (arg.compare("--graph") == 0)
			graph = true;
		else if (arg.compare("--functions") == 0)
			functions = true;
		else if (arg.compare("--allocations") == 0)
			allocations = true;
		else if (arg.compare("--help") == 0) {
			if (rank == 0) {
				cout << "Usage for WMAnalysis\n";
				cout << "Optional arguments: \n";
				cout
						<< "--graph : Prints temporal memory consumption information for graphing.\n";
				cout
						<< "--functions : Prints a function breakdown of consumption at point of high water mark.\n";
				cout
						<< "--allocations : Prints a list of 'live' allocations at point of high water mark.\n";
				cout << "--help : This help message.\n";
				cout
						<< "<Trace File Name> : The name of the file or folder (for multiple files) to trace.\n";
				cout
						<< "\tIf not specified, the folder of the most recent trace will be used.\n\n";
			}
			MPI_Finalize();
			return 0;

		} else {

			filename = arg;
			/* First check if we are processing a single file */
			if (filename.compare(filename.length() - 2, 2, ".z") == 0)
				singleFile = true;
		}

	}

	/* if single file then compute it and exit */
	if (singleFile) {
		if (rank == 0) {
			WMAnalysis *wm = new WMAnalysis(filename, graph, functions,
					allocations);
			TraceReader * tr = wm->getTraceReader();
			long mem = tr->getHWMMemory();
			long elf = tr->getStaticMem();
			cout << "Memory consumption of " << filename << " is:\n\t" << mem
					<< "(B) - Heap\n\t" << elf << "(B) - Static Memory\n";
			delete wm;
		}

		MPI_Finalize();
		return 0;
	}

	/* Perform getting count and workload distribution */
	string folder = WMUtils::extractFolder(filename);
	int count = WMUtils::countRunSize(folder);

	int start[comm], end[comm];
	memset(start, 0, comm * sizeof(int));
	memset(end, 0, comm * sizeof(int));

	int div = (int) (floor(count / comm));
	int rem = (int) (count % comm);

	int i;
	start[0] = 0;
	end[comm - 1] = count;
	for (i = 1; i < comm; i++) {
		end[i - 1] = start[i - 1] + div;
		if ((i - 1) < rem)
			end[i - 1]++;
		start[i] = end[i - 1];
	}

	long memoryArray[count];
	long elf = 0;
	for (i = start[rank]; i < end[rank]; i++) {
		string fname = WMUtils::stichFileName(folder, i);

		cout << "Processing " << fname << " on rank " << rank << "\n";

		WMAnalysis *wm = new WMAnalysis(fname, graph, functions, allocations);
		TraceReader * tr = wm->getTraceReader();
		memoryArray[i] = tr->getHWMMemory();
		cout << "Rank " << i << " Time of finish " << tr->getFinishTime()
				<< "\n";
		elf = tr->getStaticMem();

		cout << "Finished processing " << fname << " on rank " << rank << "\n";

		delete wm;
	}

	MPI_Status s;
	for (i = 1; i < comm; i++) {
		if (rank == i)
			MPI_Send(&memoryArray[start[i]], end[i] - start[i], MPI_LONG, 0, 0,
					MPI_COMM_WORLD);
		if (rank == 0)
			MPI_Recv(&memoryArray[start[i]], end[i] - start[i], MPI_LONG, i, 0,
					MPI_COMM_WORLD, &s);
	}

	if (rank == 0) {
		long max_mem = memoryArray[0], min_mem = memoryArray[0];
		int max_rank = 0, min_rank = 0;

		int i;
		for (i = 1; i < count; i++) {
			if (memoryArray[i] > max_mem) {
				max_mem = memoryArray[i];
				max_rank = i;
			}
			if (memoryArray[i] < min_mem) {
				min_mem = memoryArray[i];
				min_rank = i;
			}

		}

		double std = WMUtils::calculateStandardDeviation(memoryArray, count);

		cout << "Max mem - " << max_mem << "(B) (Rank " << max_rank << ")\n";
		cout << "Min mem - " << min_mem << "(B) (Rank " << min_rank << ")\n";
		cout << "Standard deviation - " << std << "(B) \n";
		cout << "Static memory consumption of " << elf << "(B).\n";
	}

	MPI_Finalize();
	return 0;
}

