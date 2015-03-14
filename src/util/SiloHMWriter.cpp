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
 * SiloHMWriter.cpp
 *
 *  Created on: 10 Aug 2012
 *      Author: ofjp
 */

#include "../../include/util/SiloHMWriter.h"

SiloHMWriter::SiloHMWriter(int samples, string outfile,
		vector<string>& nodeName, vector<vector<int> >& rankAssignment,
		double timeInterval) {

	/* Set up MPI variables */
	int rank = WMUtils::getMPIRank();
	int comm = WMUtils::getMPICommSize();

	this->samples = samples;
	this->outfile = outfile;
	this->node_name = nodeName;
	this->rank_assignment = rankAssignment;

	machines = nodeName.size();
	//Set up dimensions arrays for Silo
	ndims = 2;

	/* Make array for filenames, to store in the summary file */
	filenames_mesh = new char*[machines];
	filenames_value = new char*[machines];
	filenames_label = new char*[machines];

	/* Pre initialise the names arrays */
	int i;
	for (i = 0; i < machines; i++) {
		filenames_mesh[i] = new char[1000];
		filenames_value[i] = new char[1000];
		filenames_label[i] = new char[1000];
	}

	/* Calculate dimensions */
	calculateDimensions();
	calculateProcPositions();

	/* Make the folder on rank 0 and broadcast its name */
	char fname[200];
	if (rank == 0) {
		makeFolder(outfile);
		sprintf(fname, "%s", folder.c_str());
	}
	MPI_Bcast(fname, 200, MPI_CHAR, 0, MPI_COMM_WORLD);
	folder.assign(fname);

	/* Set up folder structure */
	if (rank == 0) {

		/* Make the top level descriptor file */
		string top(folder);
		top.append("/WMHeatMap.visit");
		outfile_top.open(top.c_str());

		file_counter = 0;
		char * fname = (char *) folder.c_str();
		for (file_counter = 0; file_counter < samples; file_counter++) {
			/* Need to make a subfolder labelled by the timestep. */
			char subfolder[100];
			sprintf(subfolder, "%s/WMHeatMap.%04d", fname, file_counter);
			mkdir(subfolder, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

			/* Make timestep summary file & add to top level descriptor */
			char descriptor[100];
			/* Make relative path */
			sprintf(descriptor, "WMHeatMap.%04d/WMHeatMap.summary",
					file_counter);
			outfile_top << descriptor << "\n";
			/* Reset to absolute path */
			sprintf(descriptor, "%s/WMHeatMap.summary", subfolder);

			int i;
			for (i = 0; i < machines; i++) {

				char * name = (char *) nodeName[i].c_str();
				sprintf(filenames_mesh[i], "WMHeatMap.%s:quadmesh", name);
				sprintf(filenames_value[i], "WMHeatMap.%s:Memory", name);
				sprintf(filenames_label[i], "WMHeatMap.%s:Rank", name);
			}

			DBfile *dbfile_des = DBCreate(descriptor, DB_CLOBBER, DB_LOCAL,
					"WMTools Heatmap", DB_HDF5);

			/* Calculate time */
			double time = file_counter * timeInterval;

			/* Build the oplist */
			DBoptlist *optlist = DBMakeOptlist(2);
			DBAddOption(optlist, DBOPT_DTIME, &time);
			DBAddOption(optlist, DBOPT_CYCLE, &file_counter);

			/* Add the mesh to the summary file */
			DBPutMultimesh(dbfile_des, "quadmesh", machines, filenames_mesh,
					meshtypes_mesh, optlist);

			/* Add the data to the summary file */
			DBPutMultivar(dbfile_des, "Memory", machines, filenames_value,
					meshtypes_value, optlist);

			/* Add the labels to the summary file */
			DBPutMultivar(dbfile_des, "Rank", machines, filenames_label,
					meshtypes_label, optlist);

			/* Free the oplist */
			DBFreeOptlist(optlist);

			/* Close the Silo file. */
			DBClose(dbfile_des);
		}

	}

}

void SiloHMWriter::calculateDimensions() {
	//Calculate the dimensions of the mesh
	global_x_dim = ceil(sqrt(node_name.size()));
	global_y_dim = ceil((double) node_name.size() / global_x_dim);

	//find max machine rank count
	int max_size = 0;
	int i;
	for (i = 0; i < rank_assignment.size(); i++) {
		if (rank_assignment[i].size() > max_size)
			max_size = rank_assignment[i].size();
	}

	/* Todo: convert this to a balanced grid, not a best fit, so as to not leave empty spaces */
	local_x_dim = ceil(sqrt(max_size));
	local_y_dim = ceil((double) max_size / local_x_dim);

	local_size = local_x_dim * local_y_dim;

	/* Set up global sizes */
	mesh_dims[0] = local_x_dim + 1;
	mesh_dims[1] = local_y_dim + 1;
	zone_dims[0] = local_x_dim;
	zone_dims[1] = local_y_dim;

	/* Set up baseline meshes */
	baseline_x = new float[mesh_dims[0]];
	baseline_y = new float[mesh_dims[1]];

	for (i = 0; i < mesh_dims[0]; i++)
		baseline_x[i] = (float) i;
	for (i = 0; i < mesh_dims[1]; i++)
		baseline_y[i] = (float) i;

}

void SiloHMWriter::calculateProcPositions() {

	/* Pre-construct the array coordinates for the machines */
	x_coords = new float*[machines];
	y_coords = new float*[machines];

	/* Pre-construct the array labels for the machines */
	labels = new int*[machines];

	/* Pre-construct the mesh types for the machines */
	meshtypes_mesh = new int[machines];
	meshtypes_value = new int[machines];
	meshtypes_label = new int[machines];

	/* Loop over machines and populate arrays */
	int i;
	for (i = 0; i < machines; i++) {
		/* Init array spaces */
		x_coords[i] = new float[mesh_dims[0]];
		y_coords[i] = new float[mesh_dims[1]];
		labels[i] = new int[zone_dims[0] * zone_dims[1]];

		/* Fetch machine offset */
		int x_off, y_off;
		calculateMeshOffset(i, &x_off, &y_off);

		/* Define coords arrays */
		int j;
		for (j = 0; j < mesh_dims[0]; j++)
			x_coords[i][j] = x_off + baseline_x[j];

		for (j = 0; j < mesh_dims[1]; j++)
			y_coords[i][j] = y_off + baseline_y[j];

		/* Make label array - Initialised to -1 */
		for (j = 0; j < zone_dims[0] * zone_dims[1]; j++) {
			labels[i][j] = -1;
		}

		/* Take the label data from the array mapping */
		for (j = 0; j < rank_assignment[i].size(); j++) {
			labels[i][j] = rank_assignment[i][j];
		}

		/* Define data type variables  */
		meshtypes_mesh[i] = DB_QUAD_RECT;
		meshtypes_value[i] = DB_QUADVAR;
		meshtypes_label[i] = DB_QUADVAR;
	}

}

void SiloHMWriter::makeFolder(string name) {
	int test = 0;
	struct stat my_stat;

	/* Check to see if the folder name on its own exists, if not make it and store it. */
	if (stat(name.c_str(), &my_stat) != 0) {
		mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		folder.assign(name);
		return;
	}

	/* Folder exists so pepend an integer to make a unique file dir */
	char s[name.size() + 5];
	do {
		sprintf(s, "%s%04d", name.c_str(), test);
		test++;
	} while (stat(s, &my_stat) == 0);

	/** Make the directory and save it's name */
	mkdir(s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	folder.assign(s);
}

void SiloHMWriter::calculateMeshOffset(int machine, int *x, int *y) {
	*x = (machine % global_x_dim) * local_x_dim;
	*y = ((int) floor((double) machine / global_x_dim)) * local_y_dim;
}

void SiloHMWriter::addFullMachine(int machine, long **consumption, int samples,
		long maxHWM) {

	/* Store folder */
	char *fname = (char *) folder.c_str();

	/* Save matching silo data */
	float *coords[] = { x_coords[machine], y_coords[machine] };

	/* Name arrays */
	char sub_folder[200];
	char file_name[200];

	/* Make data array */
	double data[local_size];

	int i;
	/* Loop over the samples */
	for (i = 0; i < samples; i++) {
		/* Re-establish folder names */
		sprintf(sub_folder, "%s/WMHeatMap.%04d", fname, i);
		sprintf(file_name, "%s/WMHeatMap.%s", sub_folder,
				node_name[machine].c_str());

		/* Open the Silo file */
		DBfile *dbfile_machine = DBCreate(file_name, DB_CLOBBER, DB_LOCAL,
				"WMTools Heatmap", DB_HDF5);

		/* Write mesh to Silo file */
		DBPutQuadmesh(dbfile_machine, "quadmesh", NULL, coords, mesh_dims, 2,
				DB_FLOAT, DB_COLLINEAR, NULL);

		int k;
		/* Populate data Array */
		for (k = 0; k < rank_assignment[machine].size(); k++) {
			data[k] = consumption[rank_assignment[machine][k]][i];
			data[k] /= maxHWM;
		}

		/* Reset remaining entries */
		for (k = rank_assignment[machine].size(); k < local_size; k++) {
			data[k] = 0.0;
		}

		/* Write out the data */
		DBPutQuadvar1(dbfile_machine, "Memory", "quadmesh", data, zone_dims,
				ndims, NULL, 0, DB_DOUBLE, DB_ZONECENT, NULL);
		/* Write out the labels */
		DBPutQuadvar1(dbfile_machine, "Rank", "quadmesh", labels[machine],
				zone_dims, ndims, NULL, 0, DB_INT, DB_ZONECENT, NULL);

		DBClose(dbfile_machine);

	}

}


void SiloHMWriter::finish() {
	outfile_top.close();
}

SiloHMWriter::~SiloHMWriter(){
	int i;
	/* Free the data */
	for(i=0; i<machines; i++){
		delete [] filenames_mesh[i];
		delete [] filenames_value[i];
		delete [] filenames_label[i];
		delete [] x_coords[i];
		delete [] y_coords[i];
	}
	delete [] filenames_mesh;
	delete [] filenames_value;
	delete [] filenames_label;

	delete [] x_coords;
	delete [] y_coords;

	delete[] baseline_x;
	delete[] baseline_y;

	delete [] meshtypes_mesh;
	delete [] meshtypes_value;
	delete [] meshtypes_label;

}
