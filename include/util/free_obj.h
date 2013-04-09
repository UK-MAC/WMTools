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
	 * Empty constructor for the free object
	 */
	FreeObj() {
		alloc_pointer = -1;
		alloc_time = 0;
	}
	
	
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
	* Copy constructor for FreeObj
	* @param that The FreeObj we are copying data from 
	*/
	FreeObj(const FreeObj& that){
		alloc_pointer = that.getPointer();
		alloc_time = that.getTime();		
	}
	
	/**
	* Copy assignment operator for FreeObj
	* @param that The FreeObj we are copying data from 
	*/
	FreeObj& operator=(const FreeObj& that){
		
		if(this!=&that){
			alloc_pointer = that.getPointer();
			alloc_time = that.getTime();
		}
		return *this;
	}
	

	/**
	 * Getter for the free address
	 * @return The free pointer
	 */
	long getPointer() const{
		return alloc_pointer;
	}

	/**
	 * Getter for the free time
	 * @return The free time
	 */
	float getTime() const{
		return alloc_time;
	}
};
#endif

