#ifndef MALLOC_OBJ
#define MALLOC_OBJ

#include <stdio.h>

class MallocObj {
private:
	long alloc_pointer;
	long alloc_size;
	float alloc_time;
	int stack_id;

public:
	/**
	 * Constructor for the malloc object
	 * @param pointer The returned memory address
	 * @param size The size of the allocation
	 * @param time Time time delta since the last allocation
	 * @param id The stack ID of the call stack associated with this allocation
	 */
	MallocObj(long pointer, size_t size, float time, int id) {
		alloc_pointer = pointer;
		alloc_size = size;
		alloc_time = time;
		stack_id = id;
	}

	/**
	 * Getter for the allocation address
	 * @return The allocation pointer
	 */
	long getPointer() {
		return alloc_pointer;
	}

	/**
	 * Getter for the allocation size
	 * @return The allocation size
	 */
	long getSize() {
		return alloc_size;
	}

	/**
	 * Getter for the allocation time
	 * @return The allocation time
	 */
	float getTime() {
		return alloc_time;
	}

	/**
	 * Getter for the allocation stack ID
	 * @return The allocation stack ID
	 */
	int getStackID() {
		return stack_id;
	}
};
#endif
