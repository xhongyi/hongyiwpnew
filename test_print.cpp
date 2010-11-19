/*
 * test_print.cpp
 *
 *  Created on: Nov 19, 2010
 *      Author: xhongyi
 */

#include "auto_wp.h"
#include <iostream>
#include <string>

using namespace std;

int main () {
	unsigned int watchpoint_num;
	watchpoint_t<unsigned int, unsigned int> input;
	string input_flags;
	WatchPoint<unsigned int, unsigned int> wp_test;
	deque<watchpoint_t<unsigned int, unsigned int> >::iterator test_iter;
//	unsigned int test_addr;
	bool watch_fault;

	cout << "Please input how many watchpoint you want in the system?" << endl;
	cin >> watchpoint_num;
	for (unsigned int i = 0; i < watchpoint_num; i++) {
		cout << "start_addr = ";
		cin >> input.start_addr;
		cout << "end_addr = ";
		cin >> input.end_addr;
		cout << "flags = (r/w/rw/wr/n)";
		cin >> input_flags;
		if (input_flags == "r")
			input.flags = WA_READ;
		else if (input_flags == "w")
			input.flags = WA_WRITE;
		else if (input_flags == "wr" || input_flags == "rw")
			input.flags = WA_READ | WA_WRITE;
		else
			input.flags = 0;
		wp_test.wp.push_back(input);
	}
	wp_test.watch_print();
	cout << "The search start_addr = ";
	cin >> input.start_addr;
	cout << "The search end_addr = ";
	cin >> input.end_addr;
	cout << "THe search flags = (r/w/rw/wr/n)";
	cin >> input_flags;
	if (input_flags == "r")
		input.flags = WA_READ;
	else if (input_flags == "w")
		input.flags = WA_WRITE;
	else if (input_flags == "wr" || input_flags == "rw")
		input.flags = WA_READ | WA_WRITE;
	else
		input.flags = 0;
	watch_fault = wp_test.general_fault(input.start_addr, input.end_addr, input.flags);
	if (watch_fault)
		cout << "WatchFault!" << endl;
	else
		cout << "Not a Fault!" << endl;
	/*
	cout << "Please input the search address: ";
	cin >> test_addr;
	test_iter = search_address (test_addr, wp_test.wp);
	if(test_iter == wp_test.wp.end() )
		cout << "Address not covered!";
	else {
		cout << "start_addr = " << test_iter->start_addr;
		cout << "end_addr = " << test_iter->end_addr;
	}
	*/
	cout << "program ends" << endl;
	return 0;
}
