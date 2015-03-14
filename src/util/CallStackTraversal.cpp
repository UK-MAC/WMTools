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


#include "../../include/util/CallStackTraversal.h"

StackUnwind::StackUnwind() {
	ip = 0;
	sp = 0;
	unw_max_depth = 1000;

	#ifdef MAKE_DYNA
	walker = Walker::newWalker();
	#endif

}

vector<long> StackUnwind::fullUnwind() {

#ifdef MAKE_DYNA
	return dynaUnwind();
#endif

	unw_getcontext(&uc);
	unw_init_local(&cursor, &uc);

	vector<long> call_stack;

	/* Need to step twice to remove mention of WMTrace from the call stack */
	if (unw_step(&cursor) <= 0)
		return call_stack;
	if (unw_step(&cursor) <= 0)
		return call_stack;

	int counter = 0;
	while (unw_step(&cursor) > 0 && counter < unw_max_depth) {
		unw_get_reg(&cursor, UNW_REG_IP, &ip);
		call_stack.push_back((long) ip);
		counter++;
	}

	return call_stack;
}

#ifdef MAKE_DYNA
vector<long> StackUnwind::dynaUnwind() {
	walker->walkStack(stackwalk);

	int n = stackwalk.size();

	vector<long> ips(n, 0);
	int i;
	for (unsigned i=0; i<n; i++) {
		ips[i] = (unsigned long) stackwalk[i].getRA();
	}
	return ips;
}

#endif
