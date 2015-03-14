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
 * SerialAnalysis.cpp
 *
 *  Created on: 1 Oct 2012
 *      Author: ofjp
 */

#include "../include/WMAnalysis.h"

int main(int argc, char* argv[]) {

	bool graph = false;
	bool functions = false;
	bool allocations = false;

	bool single_file = false;
	bool time_search = false;
	double time_val = 0.0;

	/* Default to file - may fail */
	string filename("WMTrace/trace-0.z");

	for (int i = 1; i < argc; i++) {
		string arg(argv[i]);

		if (arg.compare("--graph") == 0)
			graph = true;
		else if (arg.compare("--functions") == 0)
			functions = true;
		else if (arg.compare("--allocations") == 0)
			allocations = true;
		else if (arg.compare("--time") == 0){
			time_search = true;
			i++;
			time_val =  atof(argv[i]);
		}else if (arg.compare("--help") == 0) {
			cout << "Usage for WMAnalysis\n";
			cout << "Optional arguments: \n";
			cout
					<< "--graph : Prints temporal memory consumption information for graphing.\n";
			cout
					<< "--functions : Prints a function breakdown of consumption at point of high water mark.\n";
			cout
					<< "--allocations : Prints a list of 'live' allocations at point of high water mark.\n";
			cout << "--help : This help message.\n";
			cout << "<Trace File Name> : The name of the file to trace.\n\n";
			return 0;

		} else {

			filename = arg;
			/* First check if we are processing a single file */
			if (filename.compare(filename.length() - 2, 2, ".z") != 0) {
				cout
						<< "Please specify a single .z file.\nUse --help for more usage information.\n";
			}
		}

	}

	WMAnalysis *wm = new WMAnalysis(filename, graph, functions, allocations, time_search, time_val);

	/* Extract the trace readers- to get at actual data */
	TraceReader * tr = wm->getTraceReader();
	long mem = tr->getHWMMemory();
	long elf = tr->getStaticMem();

	/* Print the result */
	cout << "Memory consumption of " << filename << " is:\n\t" << mem
			<< "(B) - Heap\n\t" << elf << "(B) - Static Memory\n";

	delete wm;

}

