/*
 * WMHeatMap.cpp
 *
 *  Created on: 9 Aug 2012
 *      Author: ofjp
 */

#include "../include/WMHeatMap.h"

WMHeatMap::WMHeatMap(string inputFolder, string outputFile, int samples, bool nosilo) {
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

	string* traceFilenames = WMUtils::getJobFileNames(inputFolder);

	/* Perform rank decomposition */
	int* start;
	int* end;
	WMUtils::decompositionConstructor(comm, count, &start, &end);


	/* Map rank of trace to rank of processing job - makes for easier lookup*/
	int* rankMap = WMUtils::reverseRankMapping(comm, count, start, end);

	/* Extract HWM and Max time */
	long max_HWM = 0;
	double max_time = 0.0;

	/* Store an array of names */
	char node_names[count][200];

	if (rank == 0)
		cerr << "Extracting HWM and Max Time\n";
	int i;
	/* Loop over individuals ranks, generating the trace reader to extract information from */
	for (i = start[rank]; i < end[rank]; i++) {

		TraceReader trx(traceFilenames[i]);
		if (trx.getFinishTime() > max_time)
			max_time = trx.getFinishTime();
		if (trx.getHWMMemory() > max_HWM)
			max_HWM = trx.getHWMMemory();

		//allocateRank(trx);
		sprintf(node_names[i], "%s",
			trx.getRunDataProcessName());
		//delete trx;

	}

	/* Distribute the result to all ranks */
	long global_maxHWM = 0;
	double global_maxTime = 0.0;

	MPI_Allreduce(&max_HWM, &global_maxHWM, 1, MPI_LONG, MPI_MAX,
			MPI_COMM_WORLD);
	MPI_Allreduce(&max_time, &global_maxTime, 1, MPI_DOUBLE, MPI_MAX,
			MPI_COMM_WORLD);

	if (rank == 0)
		cerr << "  Finished! HWM of " << global_maxHWM << "(B) Max time "
				<< global_maxTime << "(S).\n";

	if (rank == 0)
		cerr << "Broadcasting node assignment.\n";

	/* Broadcast the node name list - A more elegant way to do this should exist */
	for (i = 0; i < count; i++) {
		int root = rankMap[i];

		/* Broadcast data */
		MPI_Bcast(node_names[i], 200, MPI_CHAR, root, MPI_COMM_WORLD);

		/* Store result in internal mapping */
		allocateRank(node_names[i], i);
	}

	/* Establish Data block size and object */
	long **datapoints = new long*[count];

	if (rank == 0)
		cerr << "  Finished!\nCollecting data points.\n";

	/* Extract Data Points */
	for (i = start[rank]; i < end[rank]; i++) {
		string fname = WMUtils::stichFileName(inputFolder, i);
		datapoints[i] = TraceReader::getTraceSamples(fname, samples, global_maxTime);

	}

	if (rank == 0)
		cerr << "  Finished!\nMaking Silo file structure.\n";

	double increment = global_maxTime / (samples - 1);
	/* Make Silo folder / file structure */
	SiloHMWriter silo;
	
	if(!nosilo) silo.setSiloData(samples, outputFile, machine_names,
			rank_allocation, increment);

	if (rank == 0)
		cerr << "  Finished!\nDistributing data points.\n";

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
		cerr << "  Finished!\nWriting Silo Files.\n";
	for (i = 0; i < machine_count; i++) {
		/* Only process if I own this machine */
		if (machine_owner[i] == rank) {
			if(!nosilo) silo.addFullMachine(i, datapoints, samples, global_maxHWM);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(nosilo){
		/* Find node level HWM */
		
		int sample_it;
		int proc_it;
		int machine_it;
		
		double node_max[machine_count];
		double node_time[machine_count];
		
		for(i=0;i<machine_count;i++){
			node_max[i] =0.0;
			node_time[i] =0.0;
		}
		
		for (machine_it = 0; machine_it < machine_count; machine_it++) {
				
			long node_hwm = 0;
			int node_hwm_time = 0;
			int hwm_machine = 0;
			int m_owner = machine_owner[machine_it];
			
			/* Do I own this machine? */
			if(m_owner == rank){
				/* Loop over every sample */
				for(sample_it=0; sample_it < samples; sample_it++){
					long node_sum = 0;
					/* Loop over all ranks, then determine if you own them */
					for(proc_it=0; proc_it<count; proc_it++){
						int machine_id = machine_map[proc_it];
						if(machine_id == machine_it){
							node_sum += datapoints[proc_it][sample_it];
							//cerr << "New sum " << node_sum << "\n";
						}
					}
					/* Is the sum the biggest so far? If so store it. */
					if(node_sum > node_hwm){
						node_hwm = node_sum;
						node_hwm_time = sample_it;
						//cerr << "Total " << node_sum << "\n";
					}
				}
				
				node_max[machine_it] = node_hwm/(1024*1024);
				node_time[machine_it] = node_hwm_time*increment;
				
		
				
			}
		}
		
		MPI_Allreduce(MPI_IN_PLACE, node_max, machine_count, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
		MPI_Allreduce(MPI_IN_PLACE, node_time, machine_count, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
		
		if(rank == 0){
		cout << "\n\n==== Node leve HWM ====\n\n";
			double global_node_max = node_max[0];
			int global_node = 0;
			
			for(i=0; i<machine_count; i++){
				if(node_max[i] > global_node_max){
					global_node_max = node_max[i];
					global_node = i;
				}
				
			cout << "Node " 
				<< node_names[rank_allocation[i][0]]
				<< " of " << node_max[i] 
				<< " (MB) @ time " 
				<< node_time[i] 
				<< " (s)\n";
			}
			
			cout << "\n\nMax HWM for node " 
			<< node_names[rank_allocation[global_node][0]]
			<< " of " << node_max[global_node] 
			<< " (MB) @ time " 
			<< node_time[global_node] 
			<< " (s)\n\n";
			
		}
		
		
	}

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
	bool nosilo = false;
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
			cout << "'-nosilo' Produce proc mappings but no silo output, used to get deeper HWM data.\n";
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
		} else if (strncmp(argv[i], "-nosilo", 7) == 0) {
			nosilo = true;
		}
	}

	WMHeatMap mapWMHeatMap(input, output, samples, nosilo);
	//delete map;
	MPI_Finalize();
}
