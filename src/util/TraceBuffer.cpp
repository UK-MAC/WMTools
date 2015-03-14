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


#include "../../include/util/TraceBuffer.h"

TraceBuffer::TraceBuffer(StackMap *stackmap) {

	this->stack_map = stackmap;

	/* Initiate new data objects */
	z_comp = new Compress(WMUtils::makeFileName(true));
	elf_data = new ElfData();
	frame_data = new FrameData();

	/* Set up global string buffer */
	internal_buffer = new char[BUFFERSIZE];
	internal_string_buffer.pubsetbuf(internal_buffer, BUFFERSIZE);
	buffer_used = 0;

	/* Get data for start of trace file */
	fetchElfData();
	fetchCoreData();

	//Set up flag and placeholder for size variable
	initBuffer();

}

TraceBuffer::~TraceBuffer() {
	delete z_comp;
	delete elf_data;
	delete frame_data;
	delete[] internal_buffer;
}

void TraceBuffer::initBuffer() {
	internal_string_buffer.pubseekoff(0, ios_base::beg);
	copyFlag(frame_data->DATAFLAG);
	long data = 0;
	copyToBuffer(&data, sizeof(long));
}

void TraceBuffer::finishBuffer() {
	printBuffer();
	fetchVirtualAddresses();
	z_comp->finish();
}

int TraceBuffer::fetchCallStacks() {
	long size;
	int count;

	char * data = stack_map->getPrintQueue(&size, &count);
	if (size == 0 || count == 0)
		return 1;

	/* Set up string buffer */
	long out_size = frame_data->getStacksForward() + size;
	char * stack_array = new char[out_size];
	stringbuf out_data;
	out_data.pubsetbuf(stack_array, out_size);

	/* Extract data */
	long data_size = size + sizeof(int);
	char sf = frame_data->STACKFLAG;

	/* Write data to the buffer */
	out_data.sputn((char *) &sf, sizeof(char));
	out_data.sputn((char *) &data_size, sizeof(long));
	out_data.sputn((char *) &count, sizeof(int));
	out_data.sputn((char *) data, size);

	/* Write data to compression buffer */
	int ret = z_comp->addData(stack_array, out_size);
	//cout << "Written Stack: " << out_size << "(B) Data\n";

	/* Free buffer */
	delete[] data;
	delete[] stack_array;

	return 0;

}

int TraceBuffer::fetchCoreData() {

	/* Collect MPI information */
	int rank = WMUtils::getMPIRank();
	int comm = WMUtils::getMPICommSize();

	string name = WMUtils::getMPIProcName();
	char * name_str = (char *) name.c_str();
	int name_len = name.size() + 1;

	/* Set up string buffer */
	long out_size = frame_data->getCoresForward() + name_len;
	char * stack_array = new char[out_size];
	stringbuf out_data;
	out_data.pubsetbuf(stack_array, out_size);

	/* Set up variables */
	long data_size = 3 * sizeof(int) + name_len;
	char cf = frame_data->CORESFLAG;

	/* Copy data into string buffer */
	out_data.sputn((char *) &cf, sizeof(char));
	out_data.sputn((char *) &data_size, sizeof(long));
	out_data.sputn((char *) &rank, sizeof(int));
	out_data.sputn((char *) &comm, sizeof(int));
	out_data.sputn((char *) &name_len, sizeof(int));
	out_data.sputn((char *) name_str, name_len);

	/* Write data to compression buffer */
	int ret = z_comp->addData(stack_array, out_size);
	//cout << "Written Cores: " << out_size << "(B) Data\n";

	/* Free buffer */
	delete[] stack_array;

	return 0;

}

int TraceBuffer::fetchVirtualAddresses() {
	VirtualMemoryData *vmd = new VirtualMemoryData();
	char * vFunctions_data;
	int vFunctions_size;
	int vFunctions_count = vmd->getData(&vFunctions_data, &vFunctions_size);

	/* Set up string buffer */
	long out_size = frame_data->getVirtualForward() + vFunctions_size;
	char * stack_array = new char[out_size];
	stringbuf out_data;
	out_data.pubsetbuf(stack_array, out_size);

	/* Set up variables */
	long size = vFunctions_size + sizeof(int);
	char vf = frame_data->VIRTUALFLAG;

	/* Copy data into string buffer */
	out_data.sputn((char *) &vf, sizeof(char));
	out_data.sputn((char *) &size, sizeof(long));
	out_data.sputn((char *) &vFunctions_count, sizeof(int));
	out_data.sputn((char *) vFunctions_data, vFunctions_size);

	/* Write data to compression buffer */
	int ret = z_comp->addData(stack_array, out_size);
	//cout << "Written Virtual: " << out_size << "(B) Data\n";

	/* Free buffer */
	delete[] stack_array;
	delete[] vFunctions_data;
	delete vmd;

	return 0;

}

int TraceBuffer::copyToBuffer(void* data, long size) {
	buffer_used += internal_string_buffer.sputn((char *) data, size);
	return 0;
}

int TraceBuffer::copyFlag(const char flag) {
	buffer_used += internal_string_buffer.sputn((char *) &flag, sizeof(char));
	return 0;
}

int TraceBuffer::ensureBufferSpace(long size) {
	if (BUFFERSIZE - buffer_used <= size) {
		return printBuffer();
	}
	return 0;
}

int TraceBuffer::printBuffer() {

	fetchCallStacks();

	/* Store old current position */
	long frame_size = buffer_used - frame_data->getDataForward();

	/* Move to pos 1 and insert data */
	internal_string_buffer.pubseekoff(1, ios_base::beg);
	internal_string_buffer.sputn((char *) &frame_size, sizeof(long));

	/* Return to old position */
	internal_string_buffer.pubseekoff(buffer_used, ios_base::beg);

	int ret = z_comp->addData(internal_buffer, buffer_used);


	buffer_used = 0;
	initBuffer();

	return ret;
}

int TraceBuffer::fetchElfData() {

	long elf_mem = elf_data->getElfMem();
	char *functions;
	long elf_functions;
	int function_count = elf_data->getFunctions(&functions, &elf_functions);

	/* Set up string buffer */
	long out_size = frame_data->getElfForward() + elf_functions;
	char * stack_array = new char[out_size];
	stringbuf out_data;
	out_data.pubsetbuf(stack_array, out_size);

	/* Set up variables */
	char ef = frame_data->ELFFLAG;

	/* Copy data into string buffer */
	out_data.sputn((char *) &ef, sizeof(char));
	out_data.sputn((char *) &elf_mem, sizeof(long));
	out_data.sputn((char *) &function_count, sizeof(int));
	out_data.sputn((char *) &elf_functions, sizeof(long));
	out_data.sputn((char *) functions, elf_functions);

	/* Write data to compression buffer */
	int ret = z_comp->addData(stack_array, out_size);

	/* Free buffer */
	delete[] stack_array;
	delete[] functions;

	return 0;

}

void TraceBuffer::addMalloc(long address, float time, long allocationsize,
		int stackid) {

	ensureBufferSpace(frame_data->getMallocFrameSize());

	//Flag
	copyFlag(frame_data->MALLOCFLAG);

	//Data
	copyToBuffer(&address, sizeof(long));
	copyToBuffer(&time, sizeof(float));
	copyToBuffer(&allocationsize, sizeof(long));
	copyToBuffer(&stackid, sizeof(int));


}

void TraceBuffer::addCalloc(long address, float time, long allocationsize,
		int stackid) {
	ensureBufferSpace(frame_data->getCallocFrameSize());

	//Flag
	copyFlag(frame_data->CALLOCFLAG);

	//Data
	copyToBuffer(&address, sizeof(long));
	copyToBuffer(&time, sizeof(float));
	copyToBuffer(&allocationsize, sizeof(long));
	copyToBuffer(&stackid, sizeof(int));

}

void TraceBuffer::addRealloc(long addressold, long addressnew, float time,
		long allocationsize) {
	ensureBufferSpace(frame_data->getReallocFrameSize());

	//Flag
	copyFlag(frame_data->REALLOCFLAG);

	//Data
	copyToBuffer(&addressold, sizeof(long));
	copyToBuffer(&addressnew, sizeof(long));
	copyToBuffer(&time, sizeof(float));
	copyToBuffer(&allocationsize, sizeof(long));
}

void TraceBuffer::addFree(long address, float time) {
	ensureBufferSpace(frame_data->getFreeFrameSize());

	//Flag
	copyFlag(frame_data->FREEFLAG);

	//Data
	copyToBuffer(&address, sizeof(long));
	copyToBuffer(&time, sizeof(float));

}

void TraceBuffer::addTimer(double time){
	ensureBufferSpace(frame_data->getTimerFrameSize());

	//Flag
	copyFlag(frame_data->TIMERFLAG);

	//Data
	copyToBuffer(&time, sizeof(double));

}

