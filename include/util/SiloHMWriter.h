/*
 * SiloHMWriter.h
 *
 *  Created on: 10 Aug 2012
 *      Author: ofjp
 */

#ifndef SILOHMWRITER_H_
#define SILOHMWRITER_H_

#include "mpi.h"
#include "Util.h"

#include <vector>
#include <string>
#include <unistd.h>
#include "silo.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>

using namespace std;

/**
 * This SiloHMWriter is an object to interact with the Silo library to write WMHeatMap data to a format readable from VisIt.
 * The purpose of the class is to set up the file format and and manage to printing of data.
 *
 * We group cores by machine, and print memory consumption data at time intervals along the way.
 *
 *
 * File system structure:
 * outputdir:\n
 * 	-- WMHeatMap.visit -- Top level file containing a list of all of the time step files.\n
 * 	-- WMHeatMap.0000  -- Folder for the first timestep. \n
 * 		--WMHeatMap.summary -- Root file for this timestep.\n
 * 		--WMHeatMap.nodename -- visit file per machine, per timestep.\n
 * 	-- WMHeatMap.0001  -- Folder for the first timestep. \n
 * 		--WMHeatMap.summary -- Root file for this timestep.\n
 * 		--WMHeatMap.nodename -- visit file per machine, per timestep.\n
 * 	.................\n
 */
class SiloHMWriter {
private:
	vector<vector<int> > rank_assignment;
	vector<string> node_name;

	int samples;
	string outfile;
	string folder;

	int global_x_dim;
	int global_y_dim;

	int local_x_dim;
	int local_y_dim;

	int mesh_dims[2];
	int zone_dims[2];
	int ndims;

	int file_counter;

	int local_size;

	/* WMHeatMap.visit File */
	ofstream outfile_top;

	/* Baseline mesh X dim positions */
	float *baseline_x;

	/* Baseline mesh Y dim positions */
	float *baseline_y;

	/* The number of machines in the job */
	int machines;

	/* Coordinate arrays */
	float **x_coords;
	float **y_coords;

	/* Label array */
	int **labels;

	/* Data types */
	int *meshtypes_mesh;
	int *meshtypes_value;
	int *meshtypes_label;

	/* Make array for filenames, to store in the summary file */
	char **filenames_mesh;
	char **filenames_value;
	char **filenames_label;
	
	/* Is this an active object */
	bool populated;

	/**
	 * Make the unique folder to store the silo dumps in.
	 * @param name The input name.
	 */
	void makeFolder(string name);

	/**
	 * Calculate the mesh offsets to allow for processor mappings.
	 * Processors are grouped into machines.
	 */
	void calculateDimensions();

	/**
	 * A function to generate an array of proc labels for the main mesh.
	 */
	void calculateProcPositions();

	/**
	 * A function to calculate the mesh offset of a specific machine.
	 * @param machine The id to the machine to calculate for.
	 * @param[out] x The x dimension of the mesh to start this machine at.
	 * @param[out] y The y dimension of the mesh to start this machine at.
	 */
	void calculateMeshOffset(int machine, int *x, int *y);

public:
	SiloHMWriter(){populated = false;};
	
	/**
	 * Constructor for the SiloHMWriter object. Sets up the Silo data file.
	 *
	 * @param samples The number of samples / timesteps the file will contain.
	 * @param outfile The name of the output file to generate.
	 * @param nodeName The list of nodes used in the job.
	 * @param rankAssignment The mapping of ranks to machine.
	 * @param timeInterval The timestep between each trace event.
	 */
	SiloHMWriter(int samples, string outfile, vector<string>& nodeName,
			vector<vector<int> >& rankAssignment, double timeInterval);

	/**
	 * Deconstructor for the SiloHWM object to free the memory allocated within.
	 */
	~SiloHMWriter();
	
	
	void setSiloData(int samples, string outfile, vector<string>& nodeName,
		     vector<vector<int> >& rankAssignment, double timeInterval);

	/**
	 * This function is to add the full sample data for every rank in a machine to their respective files.
	 * @param machine The ID of the machine that this call represents.
	 * @param consumption A 2D array of memory consumption, indexed by rank and sample.
	 * @param samples The number of samples for each rank.
	 * @param maxHWM The max memory High Water Mark across the job.
	 */
	void addFullMachine(int machine, long **consumption, int samples,
			long maxHWM);

	/**
	 * Function to mark the end of the trace output.
	 * The Silo file can be written and closed.
	 */
	void finish();

};

#endif /* SILOHMWRITER_H_ */
