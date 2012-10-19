#include "../../include/util/Compress.h"

Compress::Compress(string filename) {
	in = new char[CHUNKIN];
	out = new char[CHUNKOUT];
	file_out = new char[FILEOUT];

	dest.open(filename.c_str(), ios::out | ios::binary);

	/* Set the buffersize of the output */
	dest.rdbuf()->pubsetbuf(file_out, FILEOUT);

	flush_flag = Z_NO_FLUSH;

	/* allocate deflate state */

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	int ret = deflateInit(&strm, 2); //Z_DEFAULT_COMPRESSION);
	assert(ret == Z_OK);

	finish_called = 0;

}

Compress::~Compress() {
	if (finish_called == 0)
		finish();
	delete[] in;
	delete[] out;
}

int Compress::addData(char * data, int size) {

	//Reset the variables
	size_t written_total = 0;
	size_t block_length = CHUNKIN;
	size_t compress_size;

	int z_return = 0;
	//Loop while you still have data
	for (written_total = 0; written_total < size; written_total +=
			block_length) {
		//Not enough data left for complete compression buffer
		if (written_total + CHUNKIN >= size) {
			block_length = size - written_total;
		}

		//Copy the data to the input chunk
		memcpy(in, (char *) (data + written_total), block_length);

		//Set up input / output buffers
		strm.avail_in = block_length;
		strm.next_in = (Bytef *) in;
		strm.avail_out = CHUNKOUT;
		strm.next_out = (Bytef *) out;

		//Perform deflate & record data output
		z_return = deflate(&strm, flush_flag);
		compress_size = CHUNKOUT - strm.avail_out;

		dest.write(out, compress_size);

	}

	return 0;
}

int Compress::finish() {
	if (finish_called == 1)
		return 1;
	finish_called = 1;
	flush_flag = Z_FINISH;
	FrameData *f = new FrameData();
	char fin = f->FINISHFLAG;
	int ret = addData(&fin, 1);
	deflateEnd(&strm);

	dest.flush();
	dest.close();

	return ret;
}

