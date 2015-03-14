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


#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stack>
#include <unistd.h>
#include "../include/f2c.h"

#include "../include/WMTimer.h"
#include "../include/WMTrace.h"
#include "../include/WMAnalysis.h"
#include "../include/util/TraceReader.h"


#ifndef _EXTERN_C_
#ifdef __cplusplus
#define _EXTERN_C_ extern "C"
#else /* __cplusplus */
#define _EXTERN_C_
#endif /* __cplusplus */
#endif /* _EXTERN_C_ */





WMTrace *WMT = NULL;
int fortran_init = -1;
int init_called = 0;
extern "C" {
extern __typeof (malloc) __libc_malloc;
extern __typeof (calloc) __libc_calloc;
extern __typeof (realloc) __libc_realloc;
extern __typeof (free) __libc_free;
}

_EXTERN_C_ void pmpi_init(MPI_Fint *ierr);
_EXTERN_C_ void PMPI_INIT(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init_(MPI_Fint *ierr);
_EXTERN_C_ void pmpi_init__(MPI_Fint *ierr);
static void MPI_Init_fortran_wrapper(MPI_Fint *ierr) {
    int argc = 0;
    char ** argv = NULL;
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Init(&argc, &argv);
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_INIT(MPI_Fint *ierr) {
    fortran_init = 1;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init(MPI_Fint *ierr) {
    fortran_init = 2;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init_(MPI_Fint *ierr) {
    fortran_init = 3;
    MPI_Init_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_init__(MPI_Fint *ierr) {
    fortran_init = 4;
    MPI_Init_fortran_wrapper(ierr);
}



void init(){
	if(init_called) return;
	int flag;
	MPI_Initialized(&flag);
	if(WMT == NULL && flag){
	init_called=1;
	int rank, comm;

        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm);

	WMT = new WMTrace();
        
	WMT->setRank(rank);
        WMT->setCommSize(comm);

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

        /* Barrier to ensure same start time */
        //MPI_Barrier (MPI_COMM_WORLD);
        WMT->startTracing();

	}

}


/**
 * Wrapper to MPI_Init
 *
 * @param argc Count of number of arguements
 * @param argv Pointer to arguements list
 * @return MPI_Error of function
 */
int MPI_Init(int *argc, char ***argv) {
	int ret = 0;
	
	

	switch (fortran_init) {
	case -1: ret = PMPI_Init(argc, argv);      break;
        case 1: PMPI_INIT(&ret);   break;
        case 2: pmpi_init(&ret);   break;
        case 3: pmpi_init_(&ret);  break;
        case 4: pmpi_init__(&ret); break;
        default:
            fprintf(stderr, "NO SUITABLE FORTRAN MPI_INIT BINDING\n");
            break;
        }


	
	init();
	return ret;
}



void fini(){

	WMT->finishTracing();

	/* Only post process if we have told it to */
	if (WMT->isPostProcess()) {
		WMAnalysis *wm = new WMAnalysis(WMUtils::makeFileName(), WMT->isPostProcessGraph(), WMT->isPostProcessFunctions(), false, false);
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

}

/**
 * Wrapper to MPI_Finalize
 * @return MPI_Error
 */
int MPI_Finalize() {

	fini();

	int ret = PMPI_Finalize();

	return ret;
}


static void MPI_Finalize_fortran_wrapper(MPI_Fint *ierr) {
    int _wrap_py_return_val = 0;
    _wrap_py_return_val = MPI_Finalize();
    *ierr = _wrap_py_return_val;
}

_EXTERN_C_ void MPI_FINALIZE(MPI_Fint *ierr) {
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize(MPI_Fint *ierr) {
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize_(MPI_Fint *ierr) {
    MPI_Finalize_fortran_wrapper(ierr);
}

_EXTERN_C_ void mpi_finalize__(MPI_Fint *ierr) {
    MPI_Finalize_fortran_wrapper(ierr);
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
