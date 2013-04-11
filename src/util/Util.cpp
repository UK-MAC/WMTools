#include "../../include/util/Util.h"

string WMUtils::cppDemangle(string input) {

	int status;

	/* Call external function to try to demangle */
	char * real_str = abi::__cxa_demangle(input.c_str(), 0, 0, &status);

	/* Only store the result if the correct status is returned */
	if (status == 0) {
		string str(real_str);
		delete [] real_str;
		return str;
	} else {
		string str(input);
		delete [] real_str;
		return str;
	}
}

vector<long> WMUtils::stackToVector(stack<long> input) {
	/* Make the vector to store the stack in */
	vector<long> my_vec;

	/* Loop over the stack populating the vector */
	while (!input.empty()) {
		my_vec.push_back(input.top());
		input.pop();
	}

	return my_vec;
}

string WMUtils::makeFileName(bool makeNew) {
	/* Retrieve the rank */
	int rank = getMPIRank();

	/* If rank 0 make a folder name based on root */
	string folder(WMTRACEDIR);
	if (rank == 0) {
		if (makeNew)
			folder.assign(makeUniqueFolder(folder));
		else
			folder.assign(findUniqueFolder(folder));
	}

	/* Broadcast the folder name  to all ranks */
	char folder_str[100];
	sprintf(folder_str, "%s", folder.c_str());

	/* Only perform the broadcast if we are using MPI */
#ifndef NO_MPI
	MPI_Bcast(folder_str, 100, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

	return stichFileName(folder_str, rank);
}

string WMUtils::stichFileName(string base, int rank) {
	/* Group rank to folder name to make a filename */
	stringstream file;
	file << base << "/trace-" << rank << ".z";

	return file.str();
}

#ifndef NO_MPI
int WMUtils::getMPIRank() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}
#else
int WMUtils::getMPIRank() {
	/* Implemented without MPI use, returns 0 */
	return 0;
}
#endif

#ifndef NO_MPI
int WMUtils::getMPICommSize() {
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	return size;
}
#else
int WMUtils::getMPICommSize() {
	/* Implemented without MPI use, returns 1 */
	return 1;
}
#endif

#ifndef NO_MPI
string WMUtils::getMPIProcName() {
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int name_len;

	/* Fetch the processor name */
	MPI_Get_processor_name(processor_name, &name_len);
	string name(processor_name);
	return name;
}
#else
string WMUtils::getMPIProcName() {
	/* Implemented without MPI use, returns "Unknown" */
	string name("Unknown");
	return name;
}
#endif

string WMUtils::stripSuffix(string filename) {
	size_t pos = filename.find_last_of('.');
	return filename.substr(0, pos);
}

string WMUtils::makeGraphFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISGRAPH);
	return prefix;
}

string WMUtils::makeFunctionsFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISFUNCTIONS);
	return prefix;
}

string WMUtils::makeAllocationsFilename(string filename) {
	string prefix = stripSuffix(filename);
	prefix.append(WMANALYSISALLOCATIONS);
	return prefix;
}

string WMUtils::extractFolder(string filename) {
	size_t pos = filename.find_last_of('/');
	return filename.substr(0, pos);
}

double WMUtils::calculateStandardDeviation(long * array, int size) {

	/* Protect against divide by 0 error */
	if (size == 1)
		return 0;

	double mean = 0.0;
	double sosd = 0.0;

	/* Calculate Mean*/
	int i;
	for (i = 0; i < size; i++)
		mean += (double) array[i];

	mean /= (double) size;

	/* Calculate sum of square deviations */
	for (i = 0; i < size; i++)
		sosd += pow((double) array[i] - mean, 2);

	/* Divide through by one less than size */
	sosd /= (double) (size - 1);

	/* Calculate & return square root */
	return sqrt(sosd);
}

string WMUtils::makeUniqueFolder(string base) {
	int test = 0;
	struct stat myStat;
	string folder(base);
	/* Check to see if the folder name on its own exists, if not make it and store it. */
	if (stat(base.c_str(), &myStat) != 0) {
		mkdir(base.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		folder.assign(base);
		return folder;
	}

	/* Folder exists so pepend an integer to make a unique file dir */
	char s[base.size() + 5];
	do {
		sprintf(s, "%s%04d", base.c_str(), test);
		test++;
	} while (stat(s, &myStat) == 0);

	/* Make the directory and save it's name */
	mkdir(s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	folder.assign(s);
	return folder;
}

string WMUtils::findUniqueFolder(string base) {
	int test = 0;
	struct stat my_stat;
	string folder(base);

	/* Folder exists so pepend an integer to make a unique file dir */
	char s[base.size() + 5];
	sprintf(s, "%s", base.c_str());
	do {
		folder.assign(s);
		sprintf(s, "%s%04d", base.c_str(), test);
		test++;
	} while (stat(s, &my_stat) == 0);

	return folder;
}

int WMUtils::countRunSize(string base) {
	int count = -1;
	struct stat my_stat;
	char name[200];
	do {
		count++;
		sprintf(name, "%s/trace-%d.z", base.c_str(), count);
	} while (stat(name, &my_stat) == 0);

	return count;
}

string* WMUtils::getJobFileNames(const string& folder){
	int jobSize = countRunSize(folder);

	string* files = new string[jobSize];
	int i;
	for (i=0; i < jobSize; i++){
		files[i] = stichFileName(folder, i);
	}
	return files;
}

int WMUtils::decompositionConstructor(const int myRanks, const int theirRanks, int** start, int** end){

	int* startIndex = new int[myRanks];
	int* endIndex = new int[myRanks];

	int i;
	/* Sanitise the data arrays first */
	for( i=0; i < myRanks; i++){
		startIndex[i] = 0;
		endIndex[i] = 0;
	}

	int div = (int) (floor(theirRanks / myRanks));
	int rem = (int) (theirRanks % myRanks);


	startIndex[0] = 0;
	endIndex[myRanks - 1] = theirRanks;
	for (i = 1; i < myRanks; i++) {
		endIndex[i - 1] = startIndex[i - 1] + div;
		if ((i - 1) < rem)
			endIndex[i - 1]++;
		startIndex[i] = endIndex[i - 1];
	}

	*start = startIndex;
	*end = endIndex;
	return 0;
}


int* WMUtils::reverseRankMapping(const int myRanks, const int theirRanks, int* start, int* end){
	int* map = new int[theirRanks];

	int i;
	for(i=0; i < myRanks; i++){
		int j;
		for(j=start[i]; j < end[i]; j++){
			map[j] = i;
		}
	}

	return map;
}



int WMUtils::getMaxMemIndex(const long* HWM, const int traceCount){

	if(traceCount <= 0)
		return -1;
	/* Assume 0 to start with */
	int index = 0;
	long maxMem = HWM[0];
	int i;
	for(i=1; i<traceCount; i++){
		//cerr << "Considering " << HWM[i] << " against " << maxMem << "\n";
		if(HWM[i] > maxMem){
			maxMem = HWM[i];
			index=i;
		}
	}
	return index;
}

int WMUtils::getMaxTimeIndex(const double* times, const int traceCount){
	if(traceCount <= 0)
		return -1;
	/* Assume 0 to start with */
	int index = 0;
	double maxTime = times[0];
	int i;
	for(i=1; i<traceCount; i++){
		if(times[i] > maxTime){
			maxTime = times[i];
			index=i;
		}
	}
	return index;

}

int WMUtils::getUniqueJobNodes(string* names, const int traceCount, const int comm, const int* start, const int* stop, string** uniqueNodes, int** nodeNameMap){


	/* First we need to merge the lists - Only MPI */
	#ifndef NO_MPI
		int root;
		for(root=0; root < comm; root++){
			int index;
			for(index=start[root]; index<stop[root]; index++){
				char* name = new char[100];
				sprintf(name, "%s", names[index].c_str());
				MPI_Bcast(name, 100, MPI_CHAR, root, MPI_COMM_WORLD);
				names[index].assign(name);
				delete[] name;
			}
		}
	#endif

	list<string> uniques;
	int nameIndex;
	for(nameIndex = 0; nameIndex < traceCount; nameIndex++){
		uniques.push_back(names[nameIndex]);
	}
	uniques.sort();
	uniques.unique();

	int uniquesSize = uniques.size();
	string* uniqueNodeNames = new string[uniquesSize];
	int* mapping = new int[traceCount];

	/* Convert list to string* */
	int uniqueIndex = 0;
	for (list<string>::iterator it=uniques.begin(); it!=uniques.end(); ++it){
		uniqueNodeNames[uniqueIndex].assign(*it);
		uniqueIndex++;
	}

	/* Make the mapping for each ranks node name to the unique node name position */
	//int nameIndex;
	for(nameIndex = 0; nameIndex < traceCount; nameIndex++){
		uniqueIndex = 0;
		bool found = false;
		for (list<string>::iterator it=uniques.begin(); it!=uniques.end() && !found; ++it){
			if(names[nameIndex].compare(*it)==0){
				mapping[nameIndex] = uniqueIndex;
			}
			uniqueIndex++;
		}

	}

	/* Assign the return arrays */
	*nodeNameMap = mapping;
	*uniqueNodes = uniqueNodeNames;

	return uniquesSize;
}


int WMUtils::reductionSum(const int size, long* base, const long* input){
	int i;
	for(i=0; i<size; i++){
		base[i] += input[i];
	}
	return 0;
}
