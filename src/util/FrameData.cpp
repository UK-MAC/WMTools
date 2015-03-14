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


#include "../../include/util/FrameData.h"


FrameData::FrameData() {
	//Frame sizes
	malloc_frame_size = sizeof(char) + (2 * sizeof(long)) + sizeof(int)
			+ sizeof(float);
	calloc_frame_size = sizeof(char) + (2 * sizeof(long)) + sizeof(int)
			+ sizeof(float);
	realloc_frame_size = sizeof(char) + (3 * sizeof(long)) + sizeof(float);
	free_frame_size = sizeof(char) + sizeof(long) + sizeof(float);

	timer_frame_size = sizeof(char) + sizeof(double);

	data_forward = sizeof(char) + sizeof(long);
	elf_forward = sizeof(char) + (2 * sizeof(long)) + sizeof(int);
	virtual_forward = sizeof(char) + sizeof(long) + sizeof(int);
	stacks_forward = sizeof(char) + sizeof(long) + sizeof(int);
	cores_forward = sizeof(char) + sizeof(long) + (3 * sizeof(int));

}
