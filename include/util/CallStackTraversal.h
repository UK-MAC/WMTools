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


#ifndef CALLSTACK
#define CALLSTACK

#include <libunwind.h>
#include <vector>
#include <iostream>

#ifdef MAKE_DYNA

#include "frame.h"
#include "walker.h"

using namespace Dyninst::Stackwalker;

#endif

using namespace std;

class StackUnwind {
private:
	unw_cursor_t cursor;
	unw_context_t uc;
	unw_word_t ip, sp;
	int unw_max_depth;

#ifdef MAKE_DYNA
	Walker *walker;
	vector<Frame> stackwalk;

	/**
	 * A function to perform stack unwinding through the dyna stackwalker API.
	 * Allows us to switch into the thunk stack walker.
	 *
	 * @return The call stack for this function, collected through the dyna inst stackwalker API.
	 */
	vector<long> dynaUnwind();
#endif

public:
	/**
	 * Constructor for the StackUnwind object.
	 * Based on libunwind this object is designed to produce call stacks.
	 */
	StackUnwind();

	/**
	 * Perform full call stack tracing via libunwind
	 * Enforced max depth of 1000
	 *
	 * @Return stack<long> Stack of the IP's
	 */
	vector<long> fullUnwind();
};

#endif
