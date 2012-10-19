#include "../../include/util/TraceReader.h"

TraceReader::TraceReader(string filename, bool consumptionGraph,
		bool functionGraph, bool allocationGraph, bool samples, long searchID) {
	/* Set simple / complex flags */
	/* Consumption graph not considered complex as doesn't use any other info. */
	this->complex = functionGraph || allocationGraph;
	this->consumption_graph = consumptionGraph;
	this->function_graph = functionGraph;
	this->allocation_graph = allocationGraph;
	this->samples = samples;
	this->searchID = searchID;

	/* Ensure there is consistency with flags */
	if (samples)
		this->consumption_graph = true;

	quick_finish = false;
	static_mem = 0;

	/* Only make a new file if one was not provided */
	if (filename.empty())
		filename = WMUtils::makeFileName();

	/* Init the objects */
	zlib_decomp = new ZlibDecompress(filename);
	frame_data = new FrameData();
	hwm_tracker = new ConsumptionHWMTracker(filename, consumptionGraph, samples);
	f_map = new FunctionMap();
	stack_map = new StackProcessingMap();

	/* Run data is only populated later, so keep as NULL for now */
	runData = NULL;

	read();

	hwm_tracker->finish();

}

TraceReader::~TraceReader() {
	/* Destroy the object pointers */
	delete zlib_decomp;
	delete frame_data;
	delete hwm_tracker;
	delete f_map;
	delete stack_map;
	if (runData != NULL)
		delete runData;

}

void TraceReader::read() {
	char flag;
	do {
		/* Read the flag, to know what to do next */
		zlib_decomp->request(&flag, 1);

		if (flag == frame_data->FINISHFLAG) {		//End of compression stream
			break;
		} else if (flag == frame_data->ELFFLAG) {//Elf data, both static memory and functions
			processElf();
		} else if (flag == frame_data->STACKFLAG) {	//Stack ID Data
			processStacks();
		} else if (flag == frame_data->VIRTUALFLAG) {	//Process Functions
			processFunctions();
		} else if (flag == frame_data->DATAFLAG) { //Process Data events
			processEvents();
		} else if (flag == frame_data->CORESFLAG) { //Process Data events
			processCores();
		} else {
			flag = frame_data->FINISHFLAG;
		}

	} while (flag != frame_data->FINISHFLAG && !zlib_decomp->eof());


}

long TraceReader::processMalloc() {
	/* Temp variables to request data with */
	long address, size;
	float time;
	int stack;

	/* Fetch the values from the decompression object */
	zlib_decomp->request(&address, sizeof(long));
	zlib_decomp->request(&time, sizeof(float));
	zlib_decomp->request(&size, sizeof(long));
	zlib_decomp->request(&stack, sizeof(int));

	/* Make Malloc Object */
	MallocObj mal(address, size, time, stack);

	/* Add the object to the storage structure */
	return hwm_tracker->addAllocation(mal);
}

long TraceReader::processCalloc() {
	/* Temp variables to request data with */
	long address, size;
	float time;
	int stack;

	zlib_decomp->request(&address, sizeof(long));
	zlib_decomp->request(&time, sizeof(float));
	zlib_decomp->request(&size, sizeof(long));
	zlib_decomp->request(&stack, sizeof(int));

	/* Make Calloc Object (actually a malloc) */
	MallocObj mal(address, size, time, stack);

	/* Add the object to the storage structure */
	return hwm_tracker->addAllocation(mal);

}

long TraceReader::processRealloc() {
	/* Temp variables to request data with */
	long address_old, address_new, size;
	float time;

	zlib_decomp->request(&address_old, sizeof(long));
	zlib_decomp->request(&address_new, sizeof(long));
	zlib_decomp->request(&time, sizeof(float));
	zlib_decomp->request(&size, sizeof(long));

	/* Collect old malloc object, if it existed */
	MallocObj *mal = hwm_tracker->getAllocation(address_old);

	/* If object didnt exist, then act as malloc - otherwise free then malloc */
	if (mal == NULL) {
		MallocObj mal2(address_new, size, time, -1);
		return hwm_tracker->addAllocation(mal2);
	} else {
		MallocObj mal2(address_new, size, time, mal->getStackID());
		FreeObj fr(address_old, 0.0);

		hwm_tracker->addFree(fr);
		return hwm_tracker->addAllocation(mal2);
	}

}

long TraceReader::processFree() {
	/* Temp variables to request data with */
	long address;
	float time;

	zlib_decomp->request(&address, sizeof(long));
	zlib_decomp->request(&time, sizeof(float));

	FreeObj fr(address, time);

	return hwm_tracker->addFree(fr);

}

void TraceReader::processEvents() {

	long data_remaining;

	zlib_decomp->request(&data_remaining, sizeof(long));

	//If we do not need more allocations then skip them
	if (quick_finish) {
		zlib_decomp->skip(data_remaining);
		return;
	}

	//cout << "Data: " << dataRemaining << "\n";
	char flag;
	long allocID = -1;

	while (data_remaining > 0) {
		/* Read the flag to know what is next */
		zlib_decomp->request(&flag, 1);

		if (flag == frame_data->FINISHFLAG) {		//End of compression stream
			data_remaining--;
			break;
		} else if (flag == frame_data->MALLOCFLAG) {		//Malloc event
			allocID = processMalloc();
			data_remaining -= frame_data->getMallocFrameSize();
		} else if (flag == frame_data->CALLOCFLAG) {		//Calloc event
			allocID = processCalloc();
			data_remaining -= frame_data->getCallocFrameSize();
		} else if (flag == frame_data->REALLOCFLAG) {		//Realloc event
			allocID = processRealloc();
			data_remaining -= frame_data->getReallocFrameSize();
		} else if (flag == frame_data->FREEFLAG) { 		//Free event
			allocID = processFree();
			data_remaining -= frame_data->getFreeFrameSize();
		} else {
			flag = frame_data->FINISHFLAG;
			data_remaining--;
		}

		/* Check the alloc ID against the search */
		if (checkIDSearch(allocID)) {
			/* Skip any remaining data */
			zlib_decomp->skip(data_remaining);
			/* Return so as to not read any more symbols */
			return;
		}
	}

}

void TraceReader::processStacks() {

	long size;
	int count;

	zlib_decomp->request(&size, sizeof(long));

	/* If we do not need more allocations then skip them */
	if (quick_finish || !complex) {
		zlib_decomp->skip(size);
		return;
	}

	/* Get the count, for the number for stacks in thsi frame */
	zlib_decomp->request(&count, sizeof(int));

	int i;

	/* Loop over stackes, decompressing and storing each in turn */
	for (i = 0; i < count; i++) {
		int stack_ID;
		int stack_size;

		zlib_decomp->request(&stack_ID, sizeof(int));
		zlib_decomp->request(&stack_size, sizeof(int));

		long stack[stack_size];

		zlib_decomp->request(stack, sizeof(long) * stack_size);

		stack_map->addCallStack(stack_ID, stack_size, stack);
	}


}

void TraceReader::processElf() {
	long static_mem, function_size;
	int elf_functions;

	zlib_decomp->request(&static_mem, sizeof(long));
	zlib_decomp->request(&elf_functions, sizeof(int));
	zlib_decomp->request(&function_size, sizeof(long));

	/* Pass the static mem through to the graph */
	hwm_tracker->setElf(static_mem);
	this->static_mem = static_mem;

	/* If we do not need more allocations then skip them */
	if (quick_finish || !complex) {
		zlib_decomp->skip(function_size);
		return;
	}

	/* Use _init and _end as markers to start and stop function recording */

	int i;
	long prev_addr = -1;
	string prev_name;
	bool started = false;

	/* Loop over functions, de-compressing and storing them */
	for (i = 0; i < elf_functions; i++) {
		long function_address;
		int function_name_length;

		zlib_decomp->request(&function_address, sizeof(long));
		zlib_decomp->request(&function_name_length, sizeof(int));

		char name[function_name_length];
		zlib_decomp->request(name, function_name_length);

		/* If we have started then look for end, or process otherwise look for start */
		if (started) {
			if (strcmp(name, "_end") == 0) {
				started = false;
			}
			f_map->addDynamicFunction(prev_addr, function_address,
					WMUtils::cppDemangle(prev_name));
			prev_addr = function_address;
			prev_name = name;
		} else {
			if (strcmp(name, "_init") == 0) {
				started = true;
			}
		}

	}

}

void TraceReader::processCores() {

	int frame_size, rank, comm, name_len;

	zlib_decomp->request(&frame_size, sizeof(long));
	zlib_decomp->request(&rank, sizeof(int));
	zlib_decomp->request(&comm, sizeof(int));
	zlib_decomp->request(&name_len, sizeof(int));

	char name[name_len];
	zlib_decomp->request(name, name_len);

	/* Add rank info to graph */
	hwm_tracker->setRank(rank);

	/* Make new RunData object with this information */
	runData = new RunData(rank, comm, name, name_len);


}

void TraceReader::processFunctions() {

	long frame_size;
	int function_count;

	zlib_decomp->request(&frame_size, sizeof(long));

	/* If we do not need more allocations then skip them */
	if (!complex) {
		zlib_decomp->skip(frame_size);
		return;
	}

	/* Collect the number of functions */
	zlib_decomp->request(&function_count, sizeof(int));


	int i;

	/* Loop over functions de-compressing and processing them by adding to the map */
	for (i = 0; i < function_count; i++) {
		long start, end;
		int name_size;

		zlib_decomp->request(&start, sizeof(long));
		zlib_decomp->request(&end, sizeof(long));
		zlib_decomp->request(&name_size, sizeof(int));

		char name[name_size];
		zlib_decomp->request(name, name_size);

		f_map->addDynamicFunction(start, end, name);

	}

}

bool TraceReader::checkIDSearch(long id) {

	/* Look to see if we have hit the allocation of interest */
	if (!complex)
		return false;

	/* If found set the quick finish to true */
	if (id == searchID)
		quick_finish = true;

	return quick_finish;
}

vector<string> TraceReader::getCallStack(int id) {
	/* Fetch the vector of addresses from the stackMap object */
	vector<long> addresses = stack_map->getVector(id);
	int address_count = addresses.size();

	/* Make a new vector for the strings, of the same size */
	vector <string> functions(address_count);

	int i;
	/* Convert each address from a pointer to a string */
  	for (i = 0; i < address_count; i++) {
		functions[i] = f_map->getFunctionFromAddress(addresses[i]);
	}

	return functions;

}

