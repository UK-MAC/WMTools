#ifndef FUNCTIONSITE
#define FUNCTIONSITE

using namespace std;

/**
 * A data structure class to represent the memory space of a function.
 * Used to map memory addresses to function name.
 */
class FunctionSite {
private:
	/* The starting memory address of the function */
	long start_address;
	/* The ending memory address of the function */
	long end_address;
	/* The string name of the function */
	string function_name;

public:
	/**
	 * Constructor for the FunctionSite object.
	 * Takes the start and end addresses of the function, as well as the function name
	 * @param start_address The start address of the function
	 * @param end_address The end address of the function
	 * @param function_name The name of the function
	 */
	FunctionSite(long start_address, long end_address, string function_name) {
		this->start_address = start_address;
		this->end_address = end_address;
		this->function_name = function_name;
	}

	/**
	 * Getter for the end memory address of the function.
	 *
	 * @return The end memory address of the function.
	 */
	long getEndAddress() const {
		return end_address;
	}

	/**
	 * Setter for the end memory address of the function.
	 *
	 * @param endAddress The new end memory address of the function.
	 */
	void setEndAddress(long endAddress) {
		this->end_address = endAddress;
	}

	/**
	 * Getter for the function name.
	 *
	 * @return The function name.
	 */
	const string& getFunctionName() const {
		return function_name;
	}

	/**
	 * Setter for the function name.
	 *
	 * @param endAddress The new function name.
	 */
	void setFunctionName(const string& functionName) {
		this->function_name = functionName;
	}

	/**
	 * Getter for the start memory address of the function.
	 *
	 * @return The start memory address of the function.
	 */
	long getStartAddress() const {
		return start_address;
	}

	/**
	 * Setter for the start memory address of the function.
	 *
	 * @param endAddress The start end memory address of the function.
	 */
	void setStartAddress(long startAddress) {
		this->start_address = startAddress;
	}

	/**
	 * Simple function to check if a memory address sits between those defined by this object.
	 * @param address The address to check.
	 * @return If the address sits between the range of this functions object.
	 */
	bool withinRange(long address) {
		if (address >= start_address && address < end_address)
			return true;
		else
			return false;
	}

	/**
	 * Comparison function for two Function site objects.
	 * - Return 1 if the object is strictly greater than this one.
	 * - Return 0 if the memory spaces overlap.
	 * - Return -1 if the object is strictly less than this one.
	 *
	 * @param e The object to test.
	 * @return The result of the comparison.
	 */
	int compareTo(FunctionSite e) {
		if (withinRange(e.getStartAddress()) || withinRange(e.getEndAddress()))
			return 0;
		else if (e.getStartAddress() >= end_address)
			return 1;
		else
			return -1;

	}
};

#endif
