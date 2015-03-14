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


#ifndef TRACEBUFFER
#define TRACEBUFFER

#include "Util.h"

#include "Compress.h"
#include "ElfData.h"
#include "VirtualMemoryData.h"
#include "FrameData.h"
#include "StackMap.h"
#include "stdlib.h"
#include <sstream>

#include <iostream>

using namespace std;


/**
 * TraceBuffer is the internal buffer for WMTrace.
 *
 * It maintains a buffer which can be written to, which is periodically dumped to the zlib compressor.
 * It provides an interface for adding data, and automatically appends the correct frame data to the payload.
 */
class TraceBuffer {

private:
	/* Objects */
	Compress *z_comp;
	ElfData *elf_data;
	VirtualMemoryData *vmd;
	FrameData *frame_data;

	StackMap *stack_map;

	/* Buffer */
	stringbuf internal_string_buffer;
	char * internal_buffer;
	long buffer_used;

	/**
	 * Function to write elf data to the internal buffer.
	 * Uses an internal buffer which it maintains then prints to file.
	 * Takes the form of:
	 * 'E'<(long)Elf static memory><(int)Elf function count><(long)Frame size (b)>
	 * 		<(long)Function start><(int)Function name length><Function Name>
	 * 		<(long)Function start><(int)Function name length><Function Name>
	 * 		...
	 * 	@return Success of the function.
	 */
	int fetchElfData();

	/**
	 * A function to fetch the virtual address memory space functions. These differ between ranks due to memory offsets.
	 * Takes the form of:
	 * 'V'<(long) Frame size (b)><(int) Function count>
	 * 		<(long) start address><(long) End address><(int) Name length><(char *) Name>
	 * 		<(long) start address><(long) End address><(int) Name length><(char *) Name>
	 * 		...
	 * @return Success of the function.
	 */
	int fetchVirtualAddresses();

	/**
	 * Write out new call stacks from the StackMap print buffer to the zlib compression buffer.
	 * Must be written before allocation data which makes use of the stacks.
	 * If print buffer is empty, do not print frame.
	 * Takes the form of:
	 * 'S'<(long)Frame size><(int)Stack count>
	 * <(int)Stack ID><(int)Stack entries>< <(long)Address><(long)Address>... >
	 * <(int)Stack ID><(int)Stack entries>< <(long)Address><(long)Address>... >
	 * ...
	 *
	 * @return Success of the function.
	 */
	int fetchCallStacks();

	/**
	 * Write out core data about the current job, including rank, comm size, machine name and machine rank.
	 * This allows the data to be used later without having to manually remember it.
	 * Takes the form of:
	 * 'C'<(long)Frame size><(int) Rank><(int) Comm size><(int) Machine name size><(String) Machine name>
	 *
	 * @return Success of the function.
	 */
	int fetchCoreData();

	/**
	 * Function to make sure there is enough space in the buffer before adding to it.
	 *
	 * @param size The size of data waiting to be written to the buffer.
	 * @return The success of the function.
	 */
	int ensureBufferSpace(long size);

	/**
	 * A function to dump the content of the internal buffer to file through the compressor.
	 * First we print any new call stacks we have sound along the way.
	 *
	 * @return The success of the zlib compression.
	 */
	int printBuffer();

	/**
	 * A function to copy data from a pointer into the buffer.
	 * Assumes ensureBufferSpace has been called successfully.
	 *
	 * @param data The input data pointer.
	 * @param size The volume of data to copy in bytes.
	 *
	 * @return Success of the function.
	 */
	int copyToBuffer(void* data, long size);

	/**
	 * Write a flag to the output buffer to signify the start of a frame.
	 * @param flag The flag to write
	 *
	 * @return The success of the function
	 */
	int copyFlag(const char flag);

	/**
	 * Format the buffer to enable easy file output.
	 * Starts with a Data Flag followed by the size of the data in the buffer
	 */
	void initBuffer();

public:

	/**
	 * Constructor for the buffer object.
	 * Start the file by writing the ELF data.
	 */
	TraceBuffer(StackMap *stackmap);

	/**
	 * Deconstructor for the buffer object.
	 * Frees the memory of allocated objects.
	 */
	~TraceBuffer();

	/**
	 * Finish the buffer object, which in turn flushes the zlib buffer.
	 * Also print the Virtual Memory function addresses.
	 */
	void finishBuffer();

	/**
	 * Function to write a Malloc event to the buffer stream.
	 * Fixed size - mallocFrameSize
	 * Takes the form of:
	 * 'M'<(long)Address><(float)Timestamp as delta><(long)Alloc size><(int)StackID>
	 *
	 * @param[in] address The return address of the malloc
	 * @param[in] time The time since the last event
	 * @param[in] allocationsize The size of the new allocation
	 * @param[in] stackid The ID of the call stack associated with this event (or -1 of not stack)
	 *
	 */
	void addMalloc(long address, float time, long allocationsize, int stackid);

	/**
	 * Function to write a Calloc event to the buffer stream.
	 * Fixed size - callocFrameSize
	 * Takes the form of:
	 * 'C'<(long)Address><(float)Timestamp as delta><(long)Alloc size><(int)StackID>
	 *
	 * @param[in] address The return address of the calloc
	 * @param[in] time The time since the last event
	 * @param[in] allocationsize The size of the new allocation
	 * @param[in] stackid The ID of the call stack associated with this event (or -1 of not stack)
	 */
	void addCalloc(long address, float time, long allocationsize, int stackid);

	/**
	 * Function to write a realloc event to the buffer stream.
	 * Fixed size - reallocFrameSize
	 * Takes the form of:
	 * 'R'<(long)Old address><(long)New address><(float)Timestamp as delta><(long)Alloc size>
	 *
	 * @param[in] addressold The existing address of the allocation
	 * @param[in] addressnew The return address of the realloc
	 * @param[in] time The time since the last event
	 * @param[in] allocationsize The size of the new allocation
	 */
	void addRealloc(long addressold, long addressnew, float time,
			long allocationsize);

	/**
	 * Function to write a Free event to the buffer stream.
	 * Fixed size - freeFrameSize
	 * Takes the form of:
	 * 'F'<(long)Address><(float)Timestamp as delta>
	 *
	 * @param[in] address The existing address of the allocation
	 * @param[in] time The time since the last event
	 */
	void addFree(long address, float time);

	/**
	 * Function to write a timer frame to the output stream.
	 * Used to correct timer drift.
	 * Fixed frame size - timer_frame_size
	 * 'T'<(double) elapsed time>
	 *
	 * @param[in] time The elapsed time since application start.
	 */
	void addTimer(double time);

};

#endif
