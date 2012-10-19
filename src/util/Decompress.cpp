#include "../../include/util/Decompress.h"

ZlibDecompress::ZlibDecompress(string filename) {

	source.open(filename.c_str(), ifstream::in | ifstream::binary);

	/* Create buffers */
	stream_buffer_d = new char[BUFFERSIZE];
	in_d = new char[DCCHUNK];
	out_d = new char[DCCHUNK];
	buffer_remaining_d = 0;
	have_d = 0;
	space_remaining_d = BUFFERSIZE;
	curr_buffer_pos_d = stream_buffer_d;

	strm_d.zalloc = Z_NULL;
	strm_d.zfree = Z_NULL;
	strm_d.opaque = Z_NULL;
	strm_d.avail_in = 0;
	strm_d.next_in = Z_NULL;

	ret_d = inflateInit(&strm_d);
	assert(ret_d == Z_OK);
}

ZlibDecompress::~ZlibDecompress() {
	delete[] stream_buffer_d;
	delete[] in_d;
	delete[] out_d;
}

int ZlibDecompress::inflateData(void) {
	buffer_remaining_d = 0;
	space_remaining_d = BUFFERSIZE;
	curr_buffer_pos_d = stream_buffer_d;

	int t1 = source.tellg();
	source.read(in_d, DCCHUNK);
	int t2 = source.tellg();
	strm_d.avail_in = t2 - t1;

	if (strm_d.avail_in == 0)
		return -1;

	strm_d.next_in = (Bytef *) in_d;

	do {
		strm_d.avail_out = DCCHUNK;
		strm_d.next_out = (Bytef *) out_d;
		ret_d = inflate(&strm_d, Z_NO_FLUSH);

		assert(ret_d != Z_STREAM_ERROR);

		have_d = DCCHUNK - strm_d.avail_out;
		memcpy(stream_buffer_d + buffer_remaining_d, out_d, have_d);
		buffer_remaining_d += have_d;

	} while (strm_d.avail_out == 0);

	return 1;
}

int ZlibDecompress::request(void * out, size_t length) {
	size_t tmplength = length;
	char * in_buffer = (char *)out;
	/* Loop to get enough data */
	while (length > buffer_remaining_d) {

		memcpy(in_buffer, curr_buffer_pos_d, buffer_remaining_d);

		length -= buffer_remaining_d;
		in_buffer += buffer_remaining_d;
		curr_buffer_pos_d += buffer_remaining_d;
		buffer_remaining_d = 0;

		ret_d = inflateData();
		if (ret_d != 1)
			return -1; //Error

	}

	/* Finish with remaining data */
	memcpy(in_buffer, curr_buffer_pos_d, length);
	curr_buffer_pos_d += length;
	buffer_remaining_d -= length;

	return 1;
}

int ZlibDecompress::skip(size_t length) {
	while (length > buffer_remaining_d) { //Loop to get enough data

		length -= buffer_remaining_d;
		curr_buffer_pos_d += buffer_remaining_d;
		buffer_remaining_d = 0;

		ret_d = inflateData();
		if (ret_d != 1)
			return -1; //Error

	}

	curr_buffer_pos_d += length;
	buffer_remaining_d -= length;

	return 1;
}

int ZlibDecompress::resetFiles() {
	buffer_remaining_d = 0;
	space_remaining_d = BUFFERSIZE;
	curr_buffer_pos_d = stream_buffer_d;

	//fseek(source_d, 0, SEEK_SET);
	source.seekg(0, ios::beg);

	strm_d.zalloc = Z_NULL;
	strm_d.zfree = Z_NULL;
	strm_d.opaque = Z_NULL;
	strm_d.avail_in = 0;
	strm_d.next_in = Z_NULL;

	ret_d = inflateInit(&strm_d);
	return ret_d;
}

bool ZlibDecompress::eof() {
	return buffer_remaining_d == 0 && source.eof();
}
