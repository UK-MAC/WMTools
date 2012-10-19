#include "../../include/util/StackMap.h"
using namespace std;

StackMap::StackMap(bool print) {
	this->print = print;
	stack_map_ID = 0;
	print_queue_size = 0;
}

int StackMap::addStack(vector<long> new_stack) {

	/* Try to insert a new object with the vector */
	call_stack_insert = call_stack_map.insert(
			pair<vector<long>, int>(new_stack, stack_map_ID));

	/* If it was inserted, add it to the print queue, and increment */
	if (call_stack_insert.second == true) {	//new Element
		stack_map_ID++;
		if (print)
			addToPrintQueue(new_stack, call_stack_insert.first->second);
	}

	/* Return the ID of the stack */
	return call_stack_insert.first->second;

}

void StackMap::addToPrintQueue(vector<long>& newStack, int id) {
	/* Add the element to the back of the queue, and increase the size */
	print_queue.push_back(pair<int, vector<long> >(id, newStack));
	print_queue_size += (2 * sizeof(int)) + (newStack.size() * sizeof(long));
}

char *StackMap::getPrintQueue(long *size, int *count) {
	*size = print_queue_size;
	*count = print_queue.size();

	/* Set up string buffer */
	char *data = new char[print_queue_size];
	stringbuf out_data;
	out_data.pubsetbuf(data, print_queue_size);

	int buffercounter = 0;
	int i = 0;
	pair<int, vector<long> > curr;
	int vecSize;

	/* Loop over the elements in the print queue writing it to the output array */
	while (!print_queue.empty()) {
		curr = print_queue.front();	//Get from top
		print_queue.pop_front();		//Remove element
		vecSize = curr.second.size();

		/* Write data to the buffer */
		out_data.sputn((char *) &(curr.first), sizeof(int));
		out_data.sputn((char *) &vecSize, sizeof(int));

		/**
		 * Could perhaps use:
		 * memcpy((void *)(ret+buffercounter), &curr.second[0], curr.second.size() * sizeof(long));
		 * buffercounter+=curr.second.size() * sizeof(long);
		 *
		 * But this assumes that the vector is stored in contigious memory - Dangerous.
		 * Deep copy slower but safer.
		 */

		for (i = 0; i < curr.second.size(); i++)
			out_data.sputn((char *) &(curr.second[i]), sizeof(long));

	}

	print_queue_size = 0;

	return data;
}
