/*
 * WMHeatMap.cpp
 *
 *  Created on: 9 Aug 2012
 *      Author: ofjp
 */

#include "../include/WMHeatMap.h"

WMHeatMap::WMHeatMap(string inputFolder, string outputFile, int samples) {
	this->samples = samples;
	this->input_folder = inputFolder;
	this->output_file = outputFile;

	/* Set up MPI variables */
	int rank = WMUtils::getMPIRank();
	int comm = WMUtils::getMPICommSize();

	/* Get execution comm size */
	int count = WMUtils::countRunSize(inputFolder);

	/* Resize Machine map to count size */
	machine_map = new int[count];

	/* Perform rank decomposition */
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

	/* Map rank of trace to rank of processing job - makes for easier lookup*/
	int rankMap[count];
	for (i = 0; i < count; i++) {
		int k = 0;
		/* Establish Owner */
		for (k = 0; k < comm; k++) {
			if (i >= start[k] && i < end[k])
				rankMap[i] = k;
		}
	}

	/* Extract HWM and Max time */
	long max_HWM = 0;
	double max_time = 0.0;

	/* Store an array of names */
	char node_names[count][200];

	if (rank == 0)
		cout << "Extracting HWM and Max Time\n";

	/* Loop over individuals ranks, generating the trace reader to extract information from */
	for (i = start[rank]; i < end[rank]; i++) {
		string fname = WMUtils::stichFileName(inputFolder, i);

		TraceReader *trx = new TraceReader(fname);
		if (trx->getFinishTime() > max_time)
			max_time = trx->getFinishTime();
		if (trx->getHWMMemory() > max_HWM)
			max_HWM = trx->getHWMMemory();

		//allocateRank(trx);
		sprintf(node_names[i], "%s",
				trx->getRunData()->getProcNameString().c_str());
		delete trx;

	}

	/* Distribute the result to all ranks */
	long global_maxHWM = 0;
	double global_maxTime = 0.0;

	MPI_Allreduce(&max_HWM, &global_maxHWM, 1, MPI_LONG, MPI_MAX,
			MPI_COMM_WORLD);
	MPI_Allreduce(&max_time, &global_maxTime, 1, MPI_DOUBLE, MPI_MAX,
			MPI_COMM_WORLD);

	if (rank == 0)
		cout << "  Finished! HWM of " << global_maxHWM << "(B) Max time "
				<< global_maxTime << "(S).\n";

	if (rank == 0)
		cout << "Broadcasting node assignment.\n";

	/* Broadcast the node name list - A more elegant way to do this should exist */
	for (i = 0; i < count; i++) {
		int root = rankMap[i];

		/* Broadcast data */
		MPI_Bcast(node_names[i], 200, MPI_CHAR, root, MPI_COMM_WORLD);

		/* Store result in internal mapping */
		allocateRank(node_names[i], i);
	}

	/* Establish Data block size and object */
	long **datapoints = new long*[comm];

	if (rank == 0)
		cout << "  Finished!\nCollecting data points.\n";

	/* Extract Data Points */
	for (i = start[rank]; i < end[rank]; i++) {
		string fname = WMUtils::stichFileName(inputFolder, i);

		/* Extract then store the data points */
		TraceReader *trx = new TraceReader(fname, true, false, false, true);
		datapoints[i] = trx->getHeatMapSamples(samples, global_maxTime);
		delete trx;

	}

	if (rank == 0)
		cout << "  Finished!\nMaking Silo file structure.\n";

	double increment = global_maxTime / (samples - 1);
	/* Make Silo folder / file structure */
	SiloHMWriter *silo = new SiloHMWriter(samples, outputFile, machine_names,
			rank_allocation, increment);

	if (rank == 0)
		cout << "  Finished!\nDistributing data points.\n";

	/* Split up into machines */
	int machine_count = machine_names.size();
	int machine_owner[machine_count];
	/* For each machine search to see if it is owned by any rank */
	for (i = 0; i < machine_count; i++) {
		int first_rank = rank_allocation[i][0];

		/* Assign owner */
		machine_owner[i] = rankMap[first_rank];
		//if(rank == 0) cout << "Machine " << i << " With rank " << firstRank << " Owned by " << machineOwner[i] << "\n";
	}
	MPI_Barrier (MPI_COMM_WORLD);
	MPI_Status s;
	/* Share the array samples data */
	for (i = 0; i < count; i++) {
		int rank_owner = rankMap[i];
		/* Need to map from rank to machine */
		int m_owner = machine_owner[machine_map[i]];

		/* No point sending if they are the same, as the root will have the data */
		if (rank_owner != m_owner) {
			if (rank == rank_owner)
				MPI_Send(datapoints[i], samples, MPI_LONG, m_owner, i,
						MPI_COMM_WORLD);
			if (rank == m_owner) {
				datapoints[i] = new long[samples];
				MPI_Recv(datapoints[i], samples, MPI_LONG, rank_owner, i,
						MPI_COMM_WORLD, &s);
			}
		}
	}

	/* Each machine owner should now have all the data for their machine - Just need to write it out*/
	if (rank == 0)
		cout << "  Finished!\nWriting Silo Files.\n";
	for (i = 0; i < machine_count; i++) {
		/* Only process if I own this machine */
		if (machine_owner[i] == rank) {
			silo->addFullMachine(i, datapoints, samples, global_maxHWM);

		}
	}
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0)
		cout << "  Finished!\nAll done. Exiting.\n";
}

void WMHeatMap::allocateRank(char * name, int id) {
	/* If already exists then add to existing list */
	int i;
	for (i = 0; i < machine_names.size(); i++) {
		if (machine_names[i].compare(name) == 0) {
			rank_allocation[i].push_back(id);
			machine_map[id] = i;
			return;
		}
	}

	machine_map[id] = machine_names.size();
	/* Otherwise form new list and add to back */
	machine_names.push_back(name);
	vector<int> tmp;
	rank_allocation.push_back(tmp);
	rank_allocation[i].push_back(id);

}

int main(int argc, char *argv[]) {

	int rank, comm;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm);

	int samples = 1000;
	string output("WMHeatMap.hd5f");
	string input("WMTrace");

	if (argc < 2) {
		if (rank == 0) {
			cout << "WMHeatMap Usage\n";
			cout << "WMHeatMap [Options] <WMTrace Output Dir>\n\n";
			cout << "Options:\n";
			cout << "'-s=n' Number of samples for the output - Default "
					<< samples << "\n";
			cout << "'-o=n' Output filename - Default " << output << "\n";
		}
		return 1;
	}

	input.assign(argv[argc - 1]);

	int i;
	for (i = 1; i < argc - 1; i++) {
		if (strncmp(argv[i], "-s=", 3) == 0) {
			sscanf(argv[i], "-s=%d", &samples);
		} else if (strncmp(argv[i], "-o=", 3) == 0) {
			char name[200];
			sscanf(argv[i], "-o=%s", name);
			output.assign(name);
		}
	}

	WMHeatMap *map = new WMHeatMap(input, output, samples);
	delete map;
	MPI_Finalize();
}
