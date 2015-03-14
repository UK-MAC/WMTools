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


#include "../../include/util/FunctionMap.h"

string FunctionMap::getFunctionFromAddress(long address) {

	//First search cache
	function_cache_it = function_cache.find(address);
	if (function_cache_it != function_cache.end()) {
		return function_cache_it->second;
	}

	//Not found in cache, so search data structure for it.
	string fun("Unknown");

	//Make dummy object
	FunctionObj * tmp = new FunctionObj(address, address, fun, false);

	function_store_it = function_store.find(tmp);

	if (function_store_it == function_store.end()) {
		cout << "Not found in store!\n";
	} else {	//Have found an element Is it within range

		//fsIt->
		//FunctionObj * fo = (fsIt);
		if ((*function_store_it)->withinRange(address))
			fun = (string)((*function_store_it)->getName());
		else
			cout << "Found within " << (*function_store_it)->getName()
					<< " but not within range\n";
	}

	//Add it to the cache for later
	function_cache.insert(pair<long, string>(address, fun));

	//Return the function name
	return fun;

}

FunctionObj *FunctionMap::getFunctionObjFromAddress(long address) {
	string fun("Unknown");

	/* Make dummy object to search with */
	FunctionObj * tmp = new FunctionObj(address, address, fun, false);

	function_store_it = function_store.find(tmp);

	/* Check the location of the result, if it actually fond a valid entry */
	if (function_store_it == function_store.end())
		return tmp;


	/* Have found an element Is it within range */
	if ((*function_store_it)->withinRange(address))
		return (FunctionObj *) (*function_store_it);

	/* Found but not within range, so return new entry */
	return tmp;
}

void FunctionMap::addElfFunction(long start_address, long stop_address,
		string function_name) {
	/* Make a new object from the function information, and store it */
	FunctionObj * fo = new FunctionObj(start_address, stop_address, function_name,
			true);
	function_store.insert(fo);
}

void FunctionMap::addDynamicFunction(long start_address, long stop_address,
		string function_name) {
	/* Make a new object from the function information, and store it */
	FunctionObj * fo = new FunctionObj(start_address, stop_address, function_name,
			false);
	function_store.insert(fo);

}
