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
