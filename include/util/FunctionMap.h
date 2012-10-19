#ifndef FUNCTIONMAP_H_
#define FUNCTIONMAP_H_

#include <map>
#include <set>
#include <algorithm>
#include <iostream>

#include "FunctionObj.h"

using namespace std;

/**
 * FunctionMap is an object class to store a mapping between function address and name.
 *
 * This allows function lookup based on address within a range.
 * This look up requires a custom comparator to insert and search.
 *
 * A cache is also maintained to speed up searches.
 *
 */
class FunctionMap {
private:

	/** Cache for previously searched for functions.*/
	map<long, string> function_cache;
	/** Iterator for the function cache */
	map<long, string>::iterator function_cache_it;

	set<FunctionObj*, FunctionObj::comparator> function_store;
	set<FunctionObj*, FunctionObj::comparator>::iterator function_store_it;

public:
	/**
	 * Constructor for the FunctionMap object.
	 * Initialises the internal containers.
	 */
	FunctionMap() {
	}
	;

	/**
	 * Function to add an elf function to the internal data structure.
	 * @param start_address The start memory address of the function.
	 * @param stop_address The end address of the function.
	 * @param function_name The name of the function.
	 */
	void addElfFunction(long start_address, long stop_address,
			string function_name);

	/**
	 * Function to add a dynamic library function to the internal data structure.
	 * @param start_address The start memory address of the function.
	 * @param stop_address The end address of the function.
	 * @param function_name The name of the function.
	 */
	void addDynamicFunction(long start_address, long stop_address,
			string function_name);

	/**
	 * Simple function to return the function name of the function owning this address.
	 * Function returns string ("Unknown") for functions outside of range.
	 *
	 * First search the cache to see if our address has been used before.
	 * Otherwise search the full data structure.
	 *
	 * @param address The address of the function to search for.
	 * @return The name of the function residing at this address.
	 */
	string getFunctionFromAddress(long address);

	/**
	 * A function to query if the function lying within the address range came from the elf header.
	 * @param address
	 * @return
	 */
	bool isFunctionAnElfFunction(int address);

	/**
	 * A function to get the object representing the function of address provided.
	 * If the function is not found then a temporary object with a function name of "Unknown" is returned.
	 *
	 * @param address The address of the function to search for.
	 * @return The function object representing the found function.
	 */
	FunctionObj *getFunctionObjFromAddress(long address);
};

#endif
