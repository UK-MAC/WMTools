#ifndef WMANALYSIS
#define WMANALYSIS

#include "../include/util/TraceReader.h"
#include "../include/util/FunctionSiteAllocation.h"
#include "../include/util/Util.h"

#include <set>
#include <queue>

using namespace std;

/*
 * WMAnalysis is a class to manage the processing of trace files.
 * Whilst some of the post processing can be done through WMTrace this class handles the post-processing.
 *
 */
class WMAnalysis {
private:
	TraceReader *trace_reader;

	string trace_file_name;

	/* Print a graph of temporal memory consumption */
	bool allocation_graph;

	/* Print a breakdown of function consumption at point of HWM */
	bool hwm_profile;

	/* Print a list of live allocations at the point of HWM */
	bool hwm_allocations;


public:


	/**
	 * Constructor for WMAnalysis which takes a specific tracefile - generally for serial execution.
	 * @param trace_file The name of the tracefile to read process
	 * @param graph Should we produce a consumption graph
	 * @param functions Should we production a functional breakdown
	 * @param allocations Should we produce an allocations breakdown
	 * @param time_search Should we produce a functional breakdown at time time_val
	 * @param time_val Time in s of the simulation at which to dump a function breakdown
	 */
	WMAnalysis(string trace_file = "", bool graph = false,
			bool functions = false, bool allocations = false,
			bool time_search = false, double time_val=0.0);


	/*
	 * Setter for the boolean variable representing if we should print a temporal graph of consumption.
	 *
	 * @param[in] allocation_graph Should we print an allocation graph.
	 */
	void setAllocationGraph(bool allocation_graph) {
		this->allocation_graph = allocation_graph;
	}

	/*
	 * Setter for the boolean variable representing if we generate a functional breakdown at point of HWM.
	 *
	 * @param[in] hwm_allocations Should we print the functional breakdown.
	 */
	void setHwmAllocations(bool hwm_allocations) {
		this->hwm_allocations = hwm_allocations;
	}

	/*
	 * Setter for the boolean variable representing if we should print an allocation breakdown.
	 *
	 * @param[in] hwm_profile Should we print an allocation breakdown of HWM.
	 */
	void setHwmProfile(bool hwm_profile) {
		this->hwm_profile = hwm_profile;
	}

	/**
	 * A function to actually generate the HWM functional breakdown file.
	 *
	 * @param tr The trace reader containing the information about the HWM point.
	 */
	void generateFunctionBreakdown(TraceReader * tr);

	/**
	 * Function to return the actual trace reader from the analysis.
	 * @return The inner trace reader.
	 */
	TraceReader *getTraceReader() {
		return trace_reader;
	}

};

int main(int argc, char* argv[]);

#endif
