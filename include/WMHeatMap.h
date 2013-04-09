/*
 * WMHeatMap.h
 *
 *  Created on: 9 Aug 2012
 *      Author: ofjp
 */

#ifndef WMHEATMAP_H_
#define WMHEATMAP_H_

#include "mpi.h"

#include "util/TraceReader.h"
#include "util/SiloHMWriter.h"

#include <iostream>
#include <vector>
#include <string.h>
#include <algorithm>
#include <utility>

#include <stdio.h>

using namespace std;

/**
 * WMHeatMap is a class to generate a HDF5 output to be used as a Visit visualisation to map memory consumption over time.
 * The idea is to present a heat map of memory consumption per processor over the run of the job, grouped by processor.
 *
 * The binary is parallel, through MPI, and can be run as: mpirun -np x WMHeatMap -o=outdir -s=<samples> inputdir
 */
class WMHeatMap {
private:
	/* Number of samples in the output */
	int samples;
	/* The source of the data files */
	string input_folder;
	/* The desired output folder */
	string output_file;

	/* Mapping from rank to machine */
	int *machine_map;
	/* List of machine names */
	vector<string> machine_names;
	/* List of ranks for each machine */
	vector<vector<int> > rank_allocation;

	/**
	 * A function to map a rank to the machine it belongs to, maintains a two way mapping for quick reference.
	 * @param name The name of the machine.
	 * @param id The ID of the rank.
	 */
	void allocateRank(char * name, int id);

	/**
	 * A simple function to compare vectors of ints, based on the first value in the list.
	 * @param i The first vector to compare.
	 * @param j The second vector to compare.
	 * @return If i is less than j.
	 */
	bool vecComp(vector<int> i, vector<int> j) {
		return (i.front() < j.front());
	}

	//bool pairComp (pair< int, int > i, pair< int, int > j) { return (i.first<j.first); }

	struct pComp {
		bool operator()(pair<int, int> i, pair<int, int> j) {
			return (i.first < j.first);
		}
	} pairComp;

public:
	/**
	 * Constructor for the WMHeatMap object
	 * @param inputFolder The name of the folder to look for files in
	 * @param outputFile The output folder to generate files to
	 * @param samples The number of samples to generate
	 * @param nosilo Generate data but with no silo output
	 */
	WMHeatMap(string inputFolder, string outputFile, int samples, bool nosilo);

};

#endif /* WMHEATMAP_H_ */
