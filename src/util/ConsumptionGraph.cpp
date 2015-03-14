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
 * ConsumptionGraph.cpp
 *
 *  Created on: 23 Jul 2012
 *      Author: ofjp
 */

#include "../../include/util/ConsumptionGraph.h"

ConsumptionGraph::ConsumptionGraph(string filename, bool samples) {
	this->outfile_name = WMUtils::makeGraphFilename(filename);
	this->samples = samples;

	this->rank = -1;
	this->elf = 0;
	this->local_HWM = -1;
	this->global_HWM = -1;

}

ConsumptionGraph::~ConsumptionGraph() {
	consumption.clear();
}

void ConsumptionGraph::addAllocation(double time, long memory) {

	/* Only add entry if difference from last insert is greater than limit */
	if (consumption.empty() || abs(consumption.back().second - memory) > GRAPHINTERVAL)
		consumption.push_back(pair<double, long>(time, memory));
}

void ConsumptionGraph::dumpGraphToFile(double finishtime) {
	/* Ensure we don't print the graph if we are only collecting the data for samples */
	if (samples)
		return;

	double time = finishtime;
	//if (!consumption.empty())
	//	time = consumption.back().first;

	ofstream graphfile(outfile_name.c_str());

	graphfile << "echo \"\n";

	graphfile << "# Graph file from WMTools - " << outfile_name << "\n";
	graphfile << "# <time (s)> <time (%)> <Memory (MB)>\n";

	long old = 0;

	double limit = (elf + local_HWM) / GRAPHINTERVAL;

        long last_mem = 0;

	while (!consumption.empty()) {
                last_mem = consumption.front().second;
		if (abs(last_mem - old) > limit) {
			graphfile << consumption.front().first << "\t"
					<< (consumption.front().first / time) * 100 << "\t"
					<< (double) (elf + consumption.front().second)
							/ (1024 * 1024) << "\n";
			old = consumption.front().second;
		}
		consumption.pop_front();
	}
	graphfile << time << "\t100\t" << (double) (elf + last_mem)
                                                        / (1024 * 1024) << "\n";
	graphfile << "\" >> trace.plot \n";

	graphfile
			<< "#--------------------------Plot Data----------------------------\n";

	graphfile << "gnuplot <<END\n";

	graphfile << "set grid\n";
	graphfile << "set term png size 2000, 1500\n";
	graphfile << "#set size 1,0.85\n";
	graphfile << "set output \"" << outfile_name << ".png\"\n";
	graphfile << "#set format \"$%g$\"\n";

	graphfile << "#set format xy \"2^{%b}\"\n";
	graphfile << "#set format xy \"$%g$\"\n";

	graphfile << "set xlabel \"Time (%)\"\n";
	graphfile << "set ylabel \"Memory (MB)\"\n";

	graphfile << "set xrange [0:100]\n";
	graphfile << "set yrange [0:]\n";

	graphfile << "set style line 1 lt -1 lw 0.3\n";
	graphfile << "set key box linestyle 1 left top\n";

	graphfile << "#set log x 2\n";

	graphfile << "set border lw 2\n";

	graphfile << "#set key width -7.5\n";

	graphfile << "set title \"WMTools Rank " << rank << "\"\n";

	graphfile << "plot ";

	if (elf != 0)
		graphfile << (double) (elf) / (1024 * 1024)
				<< " w lines title \"Static Memory (MB)\", ";

	if (local_HWM != -1)
		graphfile << (double) (elf + local_HWM) / (1024 * 1024)
				<< " w lines title \"Rank HWM (MB)\", ";

	if (global_HWM != -1)
		graphfile << (double) (elf + global_HWM) / (1024 * 1024)
				<< " w lines title \"Job HWM (MB)\", ";

	graphfile << "\"trace.plot\"  u 2:3 w lines title \"Rank " << rank
			<< " Memory Consumption (MB)\";\n";

	graphfile << "END\n";

	graphfile << "rm trace.plot \n";

	graphfile << "exit\n";

	graphfile.flush();
	graphfile.close();


}

long * ConsumptionGraph::getHeatMapSamples(int samples, double time) {

	long * points = new long[samples];

	/* Ensure we were collecting data for samples not for graph */
	if (!samples)
		return points;

	int i;
	double increment = time / (samples - 1);
	long prevmem = consumption.front().second;
	for (i = 1; i < samples; i++) {
		double timeStep = increment * i;
		while (!consumption.empty() && consumption.front().first < timeStep) {
			consumption.pop_front();
			prevmem = consumption.front().second;
		}
		points[i] = prevmem;
	}
	return points;
}
