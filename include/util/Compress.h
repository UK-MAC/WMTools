#ifndef COMPRESS
#define COMPRESS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <zlib.h>

#include "FrameData.h"

/* Size definitions - move to util.h? */
#define CHUNKIN  655360
#define CHUNKOUT 1048576
#define FILEOUT 1048576

using namespace std;

/**
 * Compress is a zlib compression library wrapper.
 * It initialises a data buffer and streams data to file though the compressor.
 *
 * Data is compressed in chunks at output at the end of each phase.
 * Upon finishing the class will write a 'Z' finish flag to mark the end of the stream, then flush and close.
 */
class Compress {
private:
	/* Buffers */
	char *in;
	char *out;
	char *file_out;

	/* IO */
	ofstream dest;

	/* Flags */
	int flush_flag;
	int finish_called;

	/* ZLIB */
	z_stream strm;

public:
	/**
	 * Constructor for the Compress object, backed by ZLIB.
	 *
	 * @param[in] filename The string of the filename to output data to.
	 */
	Compress(string filename);

	/**
	 * Deconstructor for the compress class.
	 */
	~Compress();

	/**
	 * The function to actually compress the data.
	 * Uses a ZLIB compression object.
	 *
	 * @param[in] data The data to compress.
	 * @param[in] size The size of the data buffer in bytes.
	 *
	 * @return Success of the function.
	 *
	 */
	int addData(char * data, int size);

	/**
	 * Finish the compression stream.
	 *
	 * @return The success flag of the final write.
	 */
	int finish();
};

#endif
