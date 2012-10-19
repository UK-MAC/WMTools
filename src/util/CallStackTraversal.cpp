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
