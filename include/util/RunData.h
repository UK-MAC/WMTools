/*
 * RunData.h
 *
 *  Created on: 9 Aug 2012
 *      Author: ofjp
 */

#ifndef RUNDATA_H_
#define RUNDATA_H_

using namespace std;

/**
 * A class to store data about the run, specifically the core count and rank information.
 * This can then be used later without the user having to remember it.
 * We also capture the node name, this helps us build a run map of the job.
 */
class RunData {
private:
	int rank;
	int comm_size;
	string proc_name;
	int name_len;
public:
	/**
	 * Constructor for the RunData object. Updates values.
	 *
	 * @param rank The rank id of the run.
	 * @param comm_size The size of the comm world.
	 * @param proc_name The name of the processor.
	 * @param name_len The length of the name string.
	 */
	RunData(int rank, int comm_size, const string& proc_name, int name_len) {
		this->rank = rank;
		this->comm_size = comm_size;
		this->proc_name = proc_name;
		//this->proc_name = proc_name;
		this->name_len = name_len;
	}
	
	RunData() {
	}
	
	~RunData(){
	}
	
	void setData(int rank, int comm_size, const string& proc_name, int name_len) {
		this->rank = rank;
		this->comm_size = comm_size;
		this->proc_name = proc_name;
		//this->proc_name = proc_name;
		this->name_len = name_len;
	}

	/**
	 * Getter for the comm size.
	 *
	 * @return The comm size.
	 */
	int getCommSize() const {
		return comm_size;
	}

	/**
	 * Getter for the proc name length.
	 *
	 * @return Proc name length.
	 */
	int getNameLen() const {
		return name_len;
	}

	/**
	 * Getter for the proc name.
	 *
	 * @return Proc name.
	 */
	const char* getProcName() const {
		return proc_name.c_str();
	}

	/**
	 * Return the processor name, in the form of a c++ string object.
	 *
	 * @return The Proc name as a string.
	 */
	const string getProcNameString() const {
		//string s(proc_name);
		return proc_name;
	}

	/**
	 * Getter for the rank ID.
	 *
	 * @return Rank ID.
	 */
	int getRank() const {
		return rank;
	}

};

#endif /* RUNDATA_H_ */
