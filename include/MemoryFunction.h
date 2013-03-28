#ifndef MEMFUNCTIONS
#define MEMFUNCTIONS

#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

extern "C" {
	void *malloc(size_t size);
	void *calloc(size_t size, size_t elements);
	void *realloc(void *ptr, size_t size);
	void free(void *ptrOld);

}


int init_wmtrace(int parallel_mode);

#endif
