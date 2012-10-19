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

	/* Generate filename */
	string hwm_filename = WMUtils::makeFunctionsFilename(trace_file_name);

	/* Make file object */
	ofstream hwm_file(hwm_filename.c_str());
	long HWM = tr->getHWMMemory();

	hwm_file << "# HWM Functions file from WMTools - " << hwm_filename
			<< " HWM of " << HWM << "(B)\n";
	hwm_file << "# High Water Mark Function Breakdown\n";

	/* Establish Iterator */
	set<FunctionSiteAllocation *, FunctionSiteAllocation::comparatorMem>::reverse_iterator it =
			call_sites_mem.rbegin();

	/* Reverse iterate over call sites > orderd by size, dumping to file */
	while (it != call_sites_mem.rend()) {
		FunctionSiteAllocation * fsa = *it;
		int stackID = fsa->getStackId();
		double percentage = ((double) fsa->getMemory()) / HWM;
		percentage *= 100;

		/* Output stack ID + data */
		hwm_file << "Call Stack: " << stackID << " Allocated "
				<< fsa->getMemory() << "(B) (" << percentage << "(%) ) from "
				<< fsa->getCount() << " allocations\n";

		vector <string> functions = tr->getCallStack(stackID);
		int i;
		for (i = 0; i < functions.size(); i++) {
			/* Make indentation */
			int j;
			for (j = 0; j < i; j++)
				hwm_file << "-";

			hwm_file << functions[i] << "\n";
		}
		hwm_file << "\n\n";

		/* Move to next allocation site */
		it++;
	}

	/* Close the file */
	hwm_file.close();

}

