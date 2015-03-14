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


#include "../include/util/StackMap.h"

#include <iostream>
#include <vector>
#include <assert.h>

using namespace std;

int main(){
	long t1[] = {12342,23134,321454,123421};
	long t2[] = {12342,23134,321454,123421}; //Same as T1
	long t3[] = {432,4765,78651,45224}; //Different from T1
	long t4[] = {123421,12342,23134,321454}; //Different ordering of T1


	vector<long> t1_vec (t1, t1 + sizeof(t1_vec) / sizeof(t1_vec[0]));
	vector<long> t2_vec (t2, t2 + sizeof(t2_vec) / sizeof(t2_vec[0]));
	vector<long> t3_vec (t3, t3 + sizeof(t3_vec) / sizeof(t3_vec[0]));
	vector<long> t4_vec (t4, t4 + sizeof(t4_vec) / sizeof(t4_vec[0]));


	StackMap sm;


	int t1_res = sm.AddStack(t1_vec);
	int t2_res = sm.AddStack(t2_vec);
	int t3_res = sm.AddStack(t3_vec);
	int t4_res = sm.AddStack(t4_vec);

	assert(t2_res == t1_res);
	assert(t3_res != t1_res);
	assert(t4_res != t1_res);

	cout << "All tests passed\n";

	return 0; //Success

}
