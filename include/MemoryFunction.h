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

#endif
