#include "../watchpoint_system/page_table_wp.h"

using namespace std;

int main() {
	unsigned int watchpoint_num;
	watchpoint_t<unsigned int, unsigned int> input;
	string input_flags;
	WatchPoint<unsigned int, unsigned int> wp_test;
	WatchPoint_PT<unsigned int, unsigned int> pt_test(wp_test);
	deque<watchpoint_t<unsigned int, unsigned int> >::iterator test_iter;
	unsigned int count = 0;
//	unsigned int test_addr;
//	bool watch_fault;

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
		if (input.flags) {
			wp_test.add_watchpoint(input.start_addr, input.end_addr, input.flags);
			count += pt_test.add_watchpoint(input.start_addr, input.end_addr, input.flags);
		}
	}
	pt_test.watch_print();
	cout <<count<<" changes made."<<endl;
	
	
	cout << "Please input how many watchpoint you want to remove in the system?" << endl;
	cin >> watchpoint_num;
	for (unsigned int i = 0; i < watchpoint_num; i++) {
		cout << "Please input the start_addr you want to remove: ";
		cin >> input.start_addr;
		cout << "Please input the end_addr you want to remove: ";
		cin >> input.end_addr;
		cout << "Please input the flags you want to remove: (r/w/wr/rw/n)" << endl;
		cin >> input_flags;
		if (input_flags == "r")
			input.flags = WA_READ;
		else if (input_flags == "w")
			input.flags = WA_WRITE;
		else if (input_flags == "wr" || input_flags == "rw")
			input.flags = WA_READ | WA_WRITE;
		else
			input.flags = 0;
		if (input.flags) {
			wp_test.rm_watchpoint(input.start_addr, input.end_addr, input.flags);
			count += pt_test.rm_watchpoint(input.start_addr, input.end_addr);
		}
	}
	pt_test.watch_print();
	cout <<count<<" changes made."<<endl;
	cout << "program ends" << endl;
	return 0;
}
