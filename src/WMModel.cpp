/*
 * WMModel.cpp
 *
 *  Created on: 31 Jul 2012
 *      Author: ofjp
 */

#include "../include/WMModel.h"

WMModel::WMModel(vector<string>& filenames, vector<long *>& sizes, vector<int *>& decomps) {
	this->file_names = filenames;
	this->sizes = sizes;
	this->decomps = decomps;


	trace_count = filenames.size();

	readers.resize(trace_count);
	second_readers.resize(trace_count);
	hwm_memory.resize(trace_count);
	hwm_times.resize(trace_count);
	ranks.resize(trace_count);
	global_sizes.resize(trace_count);

	int i;


	/* Populate global sizes, if individual sizes were used */
	for(i=0;i<trace_count;i++){
		cout << filenames[i] << " Sizes : " << sizes[i][0] << "x" << sizes[i][1] << "x" << sizes[i][2] << "\n";
		global_sizes[i] = sizes[i][0] * sizes[i][1] * sizes[i][2];
	}


	/**
	 * Initialise readers, just basic to establish the HWM and time.
	 * Do we need to tell the tracer to collect all the call stacks? Or just the ones up to the HWM.
	 * Repeat the trace but only to point of HWM
	 * Need to get Call stacks. At this point?
	 */
	for (i = 0; i < trace_count; i++) {
		TraceReader *tr = new TraceReader(filenames[i]);
		readers[i] = tr;
		hwm_memory[i] = tr->getHWMMemory();
		hwm_times[i] = tr->getHWMID();
		ranks[i] = tr->getRunData()->getCommSize();
		cout << "" << i << " Ranks: " << ranks[i] << "\n";
		//Rerun the trace but stopping at the HWM.
		TraceReader *tr2 = new TraceReader(filenames[i], false, true, false,
				false, hwm_times[i]);
		second_readers[i] = tr2;

	}

	/* Establish available ratios */

	/* First assume only 2 cores */
	double rank_ratio = ((double) ranks[1]) / ranks[0];

	cout << "Rank Ratio: " << rank_ratio << "( " << ranks[1] << "/" << ranks[0]
			<< ")\n";

	/* Measure the size of the remaining comm set */
	double exclusive_rank = ((double) ranks[1] - 1) / (ranks[0] - 1);

	cout << "Exclusive Rank Ratio: " << exclusive_rank << "( " << ranks[1] - 1
			<< "/" << ranks[0] - 1 << ")\n";

	/* Measure problem size ratio */
	double problem_ratio = ((double) global_sizes[1] / ranks[1])
			/ ((double) global_sizes[0] / ranks[0]);

	cout << "Problem Ratio: " << problem_ratio << "( "
			<< ((double) global_sizes[1] / ranks[1]) << "/"
			<< ((double) global_sizes[0] / ranks[0]) << ")\n";

	/* Generate ghost cell ratio if we can */
	int decomp_size_0 = decomps[0][0] * decomps[0][1] * decomps[0][2];
	int decomp_size_1 = decomps[1][0] * decomps[1][1] * decomps[1][2];

	cout << "Ranks " << decomp_size_0 << " and " << decomp_size_1 << "\n";

	double decomp_ghost_cells = -9999;
	double ghost_ratio_1 = 0.0;
	double ghost_count_1 = 0.0;
	if(decomp_size_0 != ranks[0] || decomp_size_1 != ranks[1]){
		cout << "Rank decomposition sizes do not match, skipping decomposition comparison\n.";
	}else{

		/* Assume 1 ghost cell surrounding in each dimension - total cells including ghost */
		double ghost_0 = (ceil((double)sizes[0][0]/decomps[0][0])+2) * (ceil((double)sizes[0][1]/decomps[0][1])+2) * (ceil((double)sizes[0][2]/decomps[0][2])+2);
		double ghost_1 = (ceil((double)sizes[1][0]/decomps[1][0])+2) * (ceil((double)sizes[1][1]/decomps[1][1])+2) * (ceil((double)sizes[1][2]/decomps[1][2])+2);

		/* Ratio between the two ghost cell collections */
		decomp_ghost_cells = ghost_1 / ghost_0;

		/* Calculate the actual volume of real cells */
		double norm_0 = (ceil((double)sizes[0][0]/decomps[0][0])) * (ceil((double)sizes[0][1]/decomps[0][1])) * (ceil((double)sizes[0][2]/decomps[0][2]));
		double norm_1 = (ceil((double)sizes[1][0]/decomps[1][0])) * (ceil((double)sizes[1][1]/decomps[1][1])) * (ceil((double)sizes[1][2]/decomps[1][2]));

		/* Calculate the ratio of total with ghost to total without ghost cells */
		//double ghost_ratio_0 = norm_0/ghost_0;
		ghost_ratio_1 = norm_1/ghost_1;

		/* Calculate the ghost cell count for trace 1*/
		ghost_count_1 = ghost_1 - norm_1;

		cout << "Ghost cell ratio (" << ghost_0 << " / " << ghost_1 << ") " << decomp_ghost_cells << "\n";
		cout << "For " << filenames[1] << " there is a ratio of " << ghost_ratio_1 << " of data cells to total cells (data + ghost)\n";

	}



	/* Map call stacks of different traces to each other */
	stack_maps = new CallStackMapper(second_readers);

	/* Map the memory consumption of each call site to the corresponding mapped call site */
	memory_map = new ConsumptionMap(second_readers, stack_maps, rank_ratio,
			exclusive_rank, problem_ratio, decomp_ghost_cells, ghost_ratio_1);

	/* Get the consumption arrays */
	//double **proportional = memoryMap->getProportionalConsumption();
	long **actual = memory_map->getActualConsumption();

	/* Fetch the array size */
	int mapped_stack_size = stack_maps->getMappedSize();

	/* Loop over actual data */
	int j;
	for (j = 1; j < mapped_stack_size; j++) {

		//cout << "Mapped id " << j << " Ratio of " << proportional[1][j] << " From: " << actual[0][j] << " To: " << actual[1][j] << "\n";

		bool found = memory_map->exactMatch(0, actual[0][j], 1, actual[1][j]);
		if (!found) {
			memory_map->deepStackComparison(0, 1, j);
		}

	}

	memory_map->printFormula(ranks[1], ((double) global_sizes[1] / ranks[1]), ghost_count_1);

	return;

}

WMModel::~WMModel() {
	int i;
	for (i = 0; i < trace_count; i++) {
		delete readers[i];
		delete second_readers[i];
	}

	delete stack_maps;
	delete memory_map;
}

void WMModel::printFormula() {
	memory_map->printFormula(ranks[1], ((double) global_sizes[1] / ranks[1]));
}

void printUsage() {
	cout
			<< "WMModel is a memory consumption model generator part of the WMTools suite.\n";
	cout
			<< "WMModel takes as input any number of trace files generated from WMTrace.\n\n";
	cout
			<< "Usage: 'WMModel --tracefiles <trace1> <trace2> [<trace 3>] --problemsize <problem 1> <problem 2> [<problem 3>]'\n";
	cout << "\t'WMModel --help' prints this message\n";
}

int main(int argc, char *argv[]) {

	if (argc < 3 || strcmp(argv[1], "--help") == 0) { //need at least 3 arguments give arg[0] is name.
		printUsage();
		return 1;
	}

	/* Get position of flags */

	int trace_file_index = 0;
	int problem_size_index = 0;
	int decomp_size_index = argc;

	bool problem_size_specified = false;
	bool problem_size_2d = false;
	bool problem_size_3d = false;

	bool decomp_size_specified = false;
	bool decomp_size_2d = false;
	bool decomp_size_3d = false;


	int i;
	for (i = 1; i < argc; i++) { //Skip argv[0] as it represents name
		if (strcmp(argv[i], "--tracefiles") == 0)
			trace_file_index = i;
		else if (strcmp(argv[i], "--problemsize") == 0){
			problem_size_index = i;
			problem_size_specified = true;
		}else if (strcmp(argv[i], "--problemsize2d") == 0){
			problem_size_index = i;
			problem_size_2d = true;
		}else if (strcmp(argv[i], "--problemsize3d") == 0){
			problem_size_index = i;
			problem_size_3d = true;
		}else if (strcmp(argv[i], "--decomp2d") == 0){
			decomp_size_index = i;
			decomp_size_2d = true;
		}else if (strcmp(argv[i], "--decomp3d") == 0){
			decomp_size_index = i;
			decomp_size_3d = true;
		}
	}

	//if (trace_file_index == 0 || problem_size_index == 0 || (problem_size_index - (trace_file_index + 1)) != (argc - (problem_size_index + 1))) {
		//printUsage();
		//return 1;
	//}
	cout << "Traces: " << trace_file_index << " Sizes: " << problem_size_index << "\n";

	int files = problem_size_index - (trace_file_index + 1);

	vector < string > file_names;
	vector<long *> sizes;
	vector<int *> decomps;

	for (i = trace_file_index + 1; i < problem_size_index; i++) {
		file_names.push_back(argv[i]);
		cout << "File: " << argv[i] << "\n";
	}
	int trace_counter = 0;
	for (i = problem_size_index + 1; i < decomp_size_index; i++) {
		long *problem_data=new long[3];

		//sizes.push_back(atol(argv[i]));
		problem_data[0]=atol(argv[i]);
		if(problem_size_2d || problem_size_3d){
			i++;
			problem_data[1]=atol(argv[i]);
		}else{
			problem_data[1] = 1;
		}

		if(problem_size_3d){
			i++;
			problem_data[2]=atol(argv[i]);
		}else{
			problem_data[2] = 1;
		}

		sizes.push_back(problem_data);

		int *decomp_data=new int[3];
		decomp_data[0]=1;
		decomp_data[1]=1;
		decomp_data[2]=1;
		decomps.push_back(decomp_data);
		cout << "Size: " << argv[i] << "\n";
		trace_counter++;
	}
	trace_counter = 0;
	for (i = decomp_size_index + 1; i < argc; i++) {
		cout << "problem: " << argv[i] << "\n";
		//sizes.push_back(atol(argv[i]));
		decomps[trace_counter][0]=atoi(argv[i]);
		if(decomp_size_2d || decomp_size_3d){
			i++;
			cout << "problem: " << argv[i] << "\n";
			decomps[trace_counter][1]=atoi(argv[i]);
		}else
			decomps[trace_counter][1] = 1;

		if(decomp_size_3d){
			i++;
			cout << "problem: " << argv[i] << "\n";
			decomps[trace_counter][2]=atoi(argv[i]);
		}else
			decomps[trace_counter][2] = 1;

		
		cout << "problem _ e: " << argv[i] << "\n";
		trace_counter++;

	}



	WMModel *model = new WMModel(file_names, sizes,decomps);
	//model->printFormula();

	delete model;

	return 0;
}
