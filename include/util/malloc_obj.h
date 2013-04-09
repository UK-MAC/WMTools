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
	 * Empty constructor for MallocObj allows testing on -1 pointer
	 */
	MallocObj(){
		alloc_pointer = -1;
		alloc_size = 0;
		alloc_time = 0;
		stack_id = -1;
	}
	
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
	 * Copy constructor for MallocObj
	 * @param that The MallocObj we are copying data from 
	 */
	MallocObj(const MallocObj& that){
		alloc_pointer = that.getPointer();
		alloc_size = that.getSize();
		alloc_time = that.getTime();
		stack_id = that.getStackID();		
	}
	
	/**
	* Copy assignment operator for MallocObj
	* @param that The MallocObj we are copying data from 
	*/
	MallocObj& operator=(const MallocObj& that){
		
		if(this!=&that){
			alloc_pointer = that.getPointer();
			alloc_size = that.getSize();
			alloc_time = that.getTime();
			stack_id = that.getStackID();	
		}
		return *this;
	}

	/**
	 * Getter for the allocation address
	 * @return The allocation pointer
	 */
	long getPointer() const{
		return alloc_pointer;
	}

	/**
	 * Getter for the allocation size
	 * @return The allocation size
	 */
	long getSize() const{
		return alloc_size;
	}

	/**
	 * Getter for the allocation time
	 * @return The allocation time
	 */
	float getTime() const{
		return alloc_time;
	}

	/**
	 * Getter for the allocation stack ID
	 * @return The allocation stack ID
	 */
	int getStackID() const{
		return stack_id;
	}
};
#endif
