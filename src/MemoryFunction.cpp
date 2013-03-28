#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stack>

#include "../include/WMTimer.h"
#include "../include/WMTrace.h"
#include "../include/WMAnalysis.h"
#include "../include/util/TraceReader.h"

WMTrace *WMT = NULL;

extern "C" {
extern __typeof (malloc) __libc_malloc;
extern __typeof (calloc) __libc_calloc;
extern __typeof (realloc) __libc_realloc;
extern __typeof (free) __libc_free;
}


int init_wmtrace(int parallel_mode){
	
	WMT = new WMTrace();
	
	/* Read from config file */
	char line[256];
	/* Try ./.WMToolsConfig then ~/.WMToolsConfig if not found */
	ifstream local_file(".WMToolsConfig");
	if (!local_file.good())
		local_file.open("~/.WMToolsConfig");
	
	if (local_file.good()) {
		while (!local_file.eof()) {
			local_file.getline(line, 256);
			if (strcmp(line, "--WMTOOLSCOMPLEX") == 0) {
				WMT->setComplexTrace(true);
			} else if (strcmp(line, "--WMTOOLSPOSTPROCESS") == 0) {
				WMT->setPostProcess(true);
			} else if (strcmp(line, "--WMTOOLSPOSTPROCESSGRAPH") == 0) {
				WMT->setPostProcess(true);
				WMT->setPostProcessGraph(true);
			} else if (strcmp(line, "--WMTOOLSPOSTPROCESSFUNCTIONS") == 0) {
				WMT->setPostProcess(true);
				WMT->setPostProcessFunctions(true);
			}
		}
		
	}
	
	int rank, comm;
	
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &comm);
	
	WMT->setRank(rank);
	WMT->setCommSize(comm);
	
	/* Barrier to ensure same start time */
	MPI_Barrier (MPI_COMM_WORLD);
	
	WMT->startTracing();
	
	return 0;
	
}




/**
 * Wrapper to MPI_Init
 *
 * @param argc Count of number of arguements
 * @param argv Pointer to arguements list
 * @return MPI_Error of function
 */
int MPI_Init(int *argc, char ***argv) {

	int ret = PMPI_Init(argc, argv);
	
	init_wmtrace(MPI_THREAD_SINGLE);

	return ret;
}


/**
* Wrapper to MPI_Init_thread
*
* @param argc Count of number of arguements
* @param argv Pointer to arguements list
* @param required Level of desired thread support
* @param[out] provided Level of provided thread support
* @return MPI_Error of function
*/
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
	
	int ret = PMPI_Init_thread(argc, argv, required, provided);
	
	init_wmtrace(*provided);
	
	return ret;
}


/**
 * Wrapper to MPI_Finalize
 * @return MPI_Error
 */
int MPI_Finalize() {
	
	if(WMT == NULL) return PMPI_Finalize();

	WMT->finishTracing();

	/* Only post process if we have told it to */
	if (WMT->isPostProcess()) {
		WMAnalysis *wm = new WMAnalysis(WMUtils::makeFileName(), WMT->isPostProcessGraph(), WMT->isPostProcessFunctions(), false);
		TraceReader *tr = wm->getTraceReader();
		//TraceReader *tr = new TraceReader(WMUtils::makeFileName(),
				//WMT->isPostProcessGraph(), WMT->isPostProcessFunctions());
		long mem = tr->getHWMMemory();
		long elf = tr->getStaticMem();
		long total_mem[WMT->getCommSize()];

		/* Gather the HWMs from all ranks */
		MPI_Gather(&mem, 1, MPI_LONG, total_mem, 1, MPI_LONG, 0,
				MPI_COMM_WORLD);

		/* Print the result if only 0 */
		if (WMT->getRank() == 0) {
			long max_mem = mem, min_mem = mem;
			int max_rank = 0, min_rank = 0;

			int i;
			for (i = 1; i < WMT->getCommSize(); i++) {
				if (total_mem[i] > max_mem) {
					max_mem = total_mem[i];
					max_rank = i;
				}
				if (total_mem[i] < min_mem) {
					min_mem = total_mem[i];
					min_rank = i;
				}

			}

			double std = WMUtils::calculateStandardDeviation(total_mem,
					WMT->getCommSize());

			cout << "Max mem - " << max_mem << "(B) (Rank " << max_rank
					<< ")\n";
			cout << "Min mem - " << min_mem << "(B) (Rank " << min_rank
					<< ")\n";
			cout << "Standard deviation - " << std << "(B) \n";
			cout << "Static memory consumption of " << elf << "(B).\n";
		}

		delete wm;
	}

	int ret = PMPI_Finalize();

	return ret;
}

//extern "C" {
/**
 * Malloc wrapper
 *
 * @param size Size in bytes of requested data
 * @return The pointer to the allocated memory
 */
void *malloc(size_t size) {
	if (WMT == NULL || WMT->testActive())
		return __libc_malloc(size);
	WMT->enterActive();

	WMT->incrementMallocCounter();

	//void * ret = __libc_malloc(size);

	void * ret = WMT->traceMalloc((long) size);

	//WMTrace_active--;
	WMT->exitActive();

	return ret;
}
/**
 * Calloc wrapper
 *
 * @param size Size of each element (b)
 * @param elements Number of elements
 * @return The pointer to the allocated memory
 */
void *calloc(size_t size, size_t elements) {
	if (WMT == NULL || WMT->testActive())
		return __libc_calloc(size, elements);
	WMT->enterActive();

	WMT->incrementCallocCounter();

	//void * ret = __libc_calloc(size, elements);

	void * ret = WMT->traceCalloc((long) size, (long) elements);

	WMT->exitActive();

	return ret;
}

/**
 * Realloc Wrapper
 *
 * @param ptr Original pointer to original memory space
 * @param size The new size of the memory space
 * @return The pointer to the newly allocated memory / existing pointer
 */
void *realloc(void *ptr, size_t size) {
	if (WMT == NULL || WMT->testActive())
		return __libc_realloc(ptr, size);
	WMT->enterActive();

	WMT->incrementReallocCounter();

	//void * ret = __libc_realloc(ptr, size);

	void * ret = WMT->traceRealloc(ptr, (long) size);

	WMT->exitActive();

	return ret;
}

/**
 * Free wrapper
 *
 * @param ptr The pointer to the memory address to free
 */
void free(void *ptr) {
	if (WMT == NULL || WMT->testActive())
		return __libc_free(ptr);
	WMT->enterActive();

	WMT->incrementFreeCounter();
	WMT->traceFree(ptr);

	WMT->exitActive();

	//return __libc_free(ptr);
}
