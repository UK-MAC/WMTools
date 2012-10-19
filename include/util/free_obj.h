#ifndef FREE_OBJ
#define FREE_OBJ

#include <stdio.h>

/**
 * The FreeObj class is a glorified struct to store a free event.
 *
 * The primary purpose is to store the pointer and time of the free event.
 *
 * Similar to malloc_obj
 */
class FreeObj {
private:
	long alloc_pointer;
	float alloc_time;

public:
	/**
	 * Constructor for the free object
	 * @param pointer The address of the memory to be freed
	 * @param time The time delta since the last event
	 */
	FreeObj(long pointer, float time) {
		alloc_pointer = pointer;
		alloc_time = time;
	}

	/**
	 * Getter for the free address
	 * @return The free pointer
	 */
	long getPointer() {
		return alloc_pointer;
	}

	/**
	 * Getter for the free time
	 * @return The free time
	 */
	float getTime() {
		return alloc_time;
	}
};
#endif

