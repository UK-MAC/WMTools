#include "../include/WMAnalysis.h"


WMAnalysis::WMAnalysis(string tracefile, bool graph, bool functions,
		bool allocations) {

	/* Generate a tracefile name (from rank id) if not provided with one */
	if (tracefile.empty())
		tracefile = WMUtils::makeFileName();

	trace_file_name = tracefile;

	/* Set the flags */
	allocation_graph = graph;
	hwm_profile = functions;
	hwm_allocations = allocations;

	/* Make a new trace reader with the flags + perform first iteration */
	trace_reader = new TraceReader(tracefile, allocation_graph);

	/* If needed perform a second iteration */
	if (hwm_profile || hwm_allocations) {
		/* Extract the HWMID from first pass */
		long hwmID = trace_reader->getHWMID();
		TraceReader *secondPass = new TraceReader(tracefile, false, functions,
				allocations, false, hwmID);

		/* If required dump the graph */
		if (hwm_profile)
			generateFunctionBreakdown(secondPass);

		delete secondPass;
	}
}

void WMAnalysis::generateFunctionBreakdown(TraceReader * tr) {

	/* Extract call site allocation data */
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparator> call_sites =
			tr->getFunctionBreakdown();

	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparatorMem> call_sites_mem(
			call_sites.begin(), call_sites.end());

	/* Get High Water Mark */
	long HWM = tr->getHWMMemory();

	/* Make a temp string buffer for writing to */
	stringstream temp_stream (stringstream::in | stringstream::out);


	/* Establish Iterator */
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparatorMem>::reverse_iterator it =
			call_sites_mem.rbegin();

	/* Track MPI Memory function consumption */
	long mpi_memory = 0;
	bool mpi_found = false;
	string mpi_function_name("libmpi");
	double mpi_memory_percentage = 0;


	/* Reverse iterate over call sites > orderd by size, dumping to file */
	while (it != call_sites_mem.rend()) {

		/* Reset MPI lib found status */
		mpi_found = false;

		FunctionSiteAllocation * fsa = *it;
		int stackID = fsa->getStackId();
		double percentage = ((double) fsa->getMemory()) / HWM;
		percentage *= 100;

		/* Output stack ID + data */
		temp_stream << "Call Stack: " << stackID << " Allocated "
				<< fsa->getMemory() << "(B) (" << percentage << "(%) ) from "
				<< fsa->getCount() << " allocations\n";

		vector <string> functions = tr->getCallStack(stackID);
		int i;
		for (i = 0; i < functions.size(); i++) {
			/* Make indentation */
			int j;
			for (j = 0; j < i; j++)
				temp_stream << "-";

			temp_stream << functions[i] << "\n";

			if(!mpi_found && functions[i].find(mpi_function_name)!=string::npos){
				mpi_memory += fsa->getMemory();
				mpi_found = true;
			}

		}
		temp_stream << "\n\n";

		/* Move to next allocation site */
		it++;
	}




	/* Generate filename */
	string hwm_filename = WMUtils::makeFunctionsFilename(trace_file_name);

	/* Make file object */
	ofstream hwm_file(hwm_filename.c_str());

	hwm_file << "# HWM Functions file from WMTools - " << hwm_filename
			<< " HWM of " << HWM << "(B)\n";
	hwm_file << "#\n";

	/* Dump MPI Memory to file */
	mpi_memory_percentage = (((double) mpi_memory) / HWM)*100;
	hwm_file << "# MPI Memory summary: " << mpi_memory << "(B) (" << mpi_memory_percentage << "%) of memory attributed to MPI (" << mpi_function_name << ")\n";
	hwm_file << "#\n";
	hwm_file << "# High Water Mark Function Breakdown\n";
	hwm_file << "\n";


	/* Copy contents of temp stream buffer to final file */
	hwm_file<< temp_stream.str();


	/* Close the files */
	hwm_file.close();
}

