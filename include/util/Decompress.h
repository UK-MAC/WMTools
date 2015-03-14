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


#ifndef DECOMPRESS
#define DECOMPRESS

#include "Util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <zlib.h>
#include <iostream>

using namespace std;

/**
 * The ZLibDecompress class is a wrapper for a zlib decompression object.
 *
 * It handles the decompression window to ensure that a buffer can always be accessed.
 *
 * Data from this internal buffer can be requested, and copied out of.
 * When the buffer is near empty it automatically refills itself.
 */
class ZlibDecompress {
private:

	/* File */
	ifstream source;

	/* Variables */
	int ret_d;
	unsigned have_d;
	z_stream strm_d;

	/* Buffers */
	char *in_d;
	char *out_d;
	char *stream_buffer_d;

	/* Buffer pointers */
	size_t buffer_remaining_d;
	char *curr_buffer_pos_d;

	size_t space_remaining_d;

	/**
	 * Internal function to request more data from file.
	 * Data is stored in an internal buffer for improved I/O.
	 * @return Success of the data request
	 */
	int inflateData(void);

public:
	/**
	 * Constructor for the zlib decompression engine.
	 */
	ZlibDecompress(string filename);

	/**
	 * De constructor for the zlib decompresson engine.
	 */
	~ZlibDecompress();

	/**
	 * Function to request length bytes to be copied into the data buffer out.
	 * @param[out] out
	 * @param[in] length
	 * @return The success of the request
	 */
	int request(void * out, size_t length);

	/**
	 * Function to skip through data without storing the result to a data buffer
	 * @param[in] length The number of bytes to skip through
	 * @return The success of the skip
	 */
	int skip(size_t length);

	/**
	 * Reset the file and zlib decompression
	 * @return Success of the decompression
	 */
	int resetFiles();

	/**
	 * Quick check to see if we are at the end of the file.
	 * @return If we are at the end of the trace file.
	 */
	bool eof();

};

#endif
