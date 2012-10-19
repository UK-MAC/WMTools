#ifndef UTILFUNCTIONS
#define UTILFUNCTIONS

#ifndef NO_MPI
#include "mpi.h"
#endif

#include <string>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <iostream>
#include <cxxabi.h>
#include <vector>
#include <stack>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#define WMTRACEDIR "WMTrace"

#define WMANALYSISGRAPH ".graph"
#define WMANALYSISFUNCTIONS ".functions"
#define WMANALYSISALLOCATIONS ".allocations"

/* Define the default spacing between points on the output graph - 1kb */
#define GRAPHINTERVAL 1024
/* Define the size of the trace buffer used throughout */
#define BUFFERSIZE 33554432
/* Define the size of the decompression chunk */
#define DCCHUNK 1048576

/**
 * WMUtils is a collection of static utility functions.
 * They are designed to be accessible anywhere within the WMTools.
 */
class WMUtils {
private:

public:
	/**
	 * Empty constructor for WMUtils object.
	 * All functions should be static, thus no need for a constructor..
	 */
	WMUtils() {
	}

	/**
	 * A simple function to de-mangle a CXX function name.
	 * If error return input.
	 *
	 * @param[in] input c++ string of the function name
	 * @return The demangled string
	 */
	static string cppDemangle(string input);

	/**
	 * Convert a long stack into a long vector.
	 * Destructive on the stack, so need to pass by value.
	 *
	 * @param input The stack on longs
	 * @return The vector of longs
	 */
	static vector<long> stackToVector(stack<long> input);

	/**
	 * A simple utility function to generate a filename.
	 * Also ensures that the folder is made, by rank 0 - all ranks are then synchronised.
	 * @param [in] makeNew Should we make a new folder or locate the last opened one - default to false.
	 * @return The new filename
	 */
	static string makeFileName(bool makeNew = false);

	/**
	 * A function to generate a new file name, for analysis, based on a base folder and a rank.
	 * Unlike WMUtils::makeFileName this function does not make the folder or files.
	 * @param base The folber at the base of this filename.
	 * @param rank The rank of the trace file to generate name for.
	 * @return The generated filename.
	 */
	static string stichFileName(string base, int rank);

	/**
	 * A simple function to request the MPI rank.
	 *
	 * @return The MPI rank of the current process.
	 */
	static int getMPIRank();

	/**
	 * A simple function to request the MPI comm world size.
	 *
	 * @return The size of the MPI comm world communicator.
	 */
	static int getMPICommSize();

	/**
	 * A simple function to request the name of the MPI processor.
	 *
	 * @return The MPI processor name.
	 */
	static string getMPIProcName();

	/**
	 * A function to strip the suffix from the string filename. e.g. remove the ".z"
	 *
	 * @param filename The filename to modify.
	 * @return The resulting filename.
	 */
	static string stripSuffix(string filename);

	/**
	 * Make a filename for the graph output file.
	 * Use the original filename + the suffix recorded.
	 *
	 * @param tracefile The filename of the original trace.
	 * @return The new filename.
	 */
	static string makeGraphFilename(string tracefile);

	/**
	 * Make a filename for the functions output file.
	 * Use the original filename + the suffix recorded.
	 *
	 * @param tracefile The filename of the original trace.
	 * @return The new filename.
	 */
	static string makeFunctionsFilename(string tracefile);

	/**
	 * Make a filename for the allocations output file.
	 * Use the original filename + the suffix recorded.
	 *
	 * @param tracefile The filename of the original trace.
	 * @return The new filename.
	 */
	static string makeAllocationsFilename(string tracefile);

	/**
	 * A function to extract the base folder from a filename.
	 * @param filename The filename to extrace the folder from.
	 * @return The folder as extracted from the filename.
	 */
	static string extractFolder(string filename);

	/**
	 * Calculate a standard deviation from the numbers within the array.
	 * @param array The array of data.
	 * @param size The number of elements in the array.
	 * @return The standard deviation of the array.
	 */
	static double calculateStandardDeviation(long * array, int size);

	/**
	 * Make a folder based on a unique name derrived from the base string.
	 * @param base The string name of the folder to base the name from.
	 * @return The name of the folder generated.
	 */
	static string makeUniqueFolder(string base);

	/**
	 * Find a folder based on the last unique name derrived from the base string.
	 * @param base The string name of the folder to base the name from.
	 * @return The name of the folder found.
	 */
	static string findUniqueFolder(string base);

	/**
	 * A function to determine the number of traces in a run, based on the number of trace files in the folder.
	 * Not a perfect guarentee - best to check data stored within trace for comm size.
	 * @param base The name of the folder to check in.
	 * @return The number of trace files in the folder.
	 */
	static int countRunSize(string base);

};

#endif
