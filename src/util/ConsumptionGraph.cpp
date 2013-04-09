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

void ConsumptionGraph::dumpGraphToFile() {
	/* Ensure we don't print the graph if we are only collecting the data for samples */
	if (samples)
		return;

	double time = 0.0;
	if (!consumption.empty())
		time = consumption.back().first;

	ofstream graphfile(outfile_name.c_str());

	graphfile << "echo \"\n";

	graphfile << "# Graph file from WMTools - " << outfile_name << "\n";
	graphfile << "# <time (s)> <time (%)> <Memory (MB)>\n";

	long old = 0;

	double limit = (elf + local_HWM) / GRAPHINTERVAL;

	while (!consumption.empty()) {
		if (abs(consumption.front().second - old) > limit) {
			graphfile << consumption.front().first << "\t"
					<< (consumption.front().first / time) * 100 << "\t"
					<< (double) (elf + consumption.front().second)
							/ (1024 * 1024) << "\n";
			old = consumption.front().second;
		}
		consumption.pop_front();
	}
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

	graphfile << "#set xrange [50:21000]\n";
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

long * ConsumptionGraph::getHeatMapSamples(int sampleCount, double time) {

	long * points = new long[sampleCount];
	int i;
	for(i=0; i<sampleCount; i++){
		points[i]=0;
	}

	/* Ensure we were collecting data for samples not for graph */
	if (!samples)
		return points;

	
	double increment = time / (sampleCount - 1);
	long prevmem = consumption.front().second;
	for (i = 1; i < sampleCount; i++) {
		double timeStep = increment * i;
		while (!consumption.empty() && consumption.front().first < timeStep) {
			prevmem = consumption.front().second;
			consumption.pop_front();
		}
		points[i] = prevmem;
		//cerr << "Point " << i << " value " << prevmem << "\n";
	}
	return points;
}
