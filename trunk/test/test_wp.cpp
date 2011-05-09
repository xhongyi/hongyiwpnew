#include "../watchpoint_system/watchpoint.h"
#include <string>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	WatchPoint<unsigned int, unsigned int> wp_test;
	int thread_id, thread_id_2;
	unsigned int start, end;
	string flag;
	int result;

	ostream *output;
	if (argc==1) {
		output = &cout;
	}
	else {
		output = new ofstream(argv[1]);
	}
	
	//	Starting threads
	cout <<"---------------------------Starting Threads---------------------------"<<endl;
	cout <<"Please enter thread_id you want to start: (enter -1 to end)"<<endl;
	cin >>thread_id;
	while (thread_id != -1) {
		cout <<"Starting thread #"<<thread_id<<" ...\t\t";
		if (wp_test.start_thread(thread_id))
			cout <<"Failed to start thread #"<<thread_id<<" !!!"<<endl;
		else
			cout <<"Succeeded! "<<endl;
		cout <<"Please enter thread_id you want to start: (enter -1 to end)"<<endl;
		cin >>thread_id;
	}
	wp_test.print_threads(*output);
	//	Changing Watchpoints
	cout <<"-------------------------Changing Watchpoints-------------------------"<<endl;
	cout <<"Please enter the watchpoint you want to change: <thread_id> <start> <end> <flag(rw)> "<<endl;
	cin >>thread_id >>start >>end >>flag;
	while (thread_id != -1) {
		if (flag == "11")
			result = wp_test.set_watch(start, end, thread_id);
		else if (flag == "10")
			result = wp_test.set_read(start, end, thread_id);
		else if (flag == "01")
			result = wp_test.set_write(start, end, thread_id);
		else if (flag == "00")
			result = wp_test.rm_watch(start, end, thread_id);
		else if (flag == "1x")
			result = wp_test.update_set_read(start, end, thread_id);
		else if (flag == "x1")
			result = wp_test.update_set_write(start, end, thread_id);
		else if (flag == "0x")
			result = wp_test.rm_read(start, end, thread_id);
		else if (flag == "x0")
			result = wp_test.rm_write(start, end, thread_id);
		else {
			result = -1;
			cout <<"Invalid flag! Flag format: 11 10 01 00 1x x1 0x x0.\t";
		}
		if (!result)
			cout <<"Succeeded!"<<endl;
		else
			cout <<"Failed!"<<endl;
		cout <<"Please enter the watchpoint you want to change: <thread_id> <start> <end> <flag(rw)> "<<endl;
		cin >>thread_id >>start >>end >>flag;
	}
	wp_test.print_watchpoints(*output);
	//	Ending threads
	cout <<"----------------------------Ending Threads----------------------------"<<endl;
	cout <<"Please enter thread_id you want to end: (enter -1 to end)"<<endl;
	cin >>thread_id;
	while (thread_id != -1) {
		cout <<"Ending thread #"<<thread_id<<" ...\t\t";
		if (wp_test.end_thread(thread_id))
			cout <<"Failed to end thread #"<<thread_id<<" !!!"<<endl;
		else
			cout <<"Succeeded! "<<endl;
		cout <<"Please enter thread_id you want to end: (enter -1 to end)"<<endl;
		cin >>thread_id;
	}
	wp_test.print_threads(*output);
	//	Checking Watchpoints
	cout <<"-------------------------Checking Watchpoints-------------------------"<<endl;
	cout <<"Please enter the watchpoint you want to check: <thread_id> <start> <end> <flag(rw)> "<<endl;
	cin >>thread_id >>start >>end >>flag;
	while (thread_id != -1) {
		if (flag == "rw")
			result = wp_test.watch_fault(start, end, thread_id);
		else if (flag == "r")
			result = wp_test.read_fault(start, end, thread_id);
		else if (flag == "w")
			result = wp_test.write_fault(start, end, thread_id);
		else {
			result = false;
			cout <<"Invalid flag!!! Flag format: rw r w.\t";
		}
		if (result)
			cout <<"It is a fault!"<<endl;
		else
			cout <<endl;
		cout <<"Please enter the watchpoint you want to check: <thread_id> <start> <end> <flag(rw)> "<<endl;
		cin >>thread_id >>start >>end >>flag;
	}
	//	Comparing Watchpoints
	cout <<"------------------------Comparing Watchpoints-------------------------"<<endl;
	cout <<"Please enter the watchpoint you want to compare: <thread_id1> <thread_id2> <flag1(r/w)flag2(r/w)> "<<endl;
	cin >>thread_id >>thread_id_2 >>flag;
	while (thread_id != -1) {
		if (flag == "rr")
			result = wp_test.compare_rr(thread_id, thread_id_2);
		else if (flag == "rw")
			result = wp_test.compare_rw(thread_id, thread_id_2);
		else if (flag == "wr")
			result = wp_test.compare_wr(thread_id, thread_id_2);
		else if (flag == "ww")
			result = wp_test.compare_ww(thread_id, thread_id_2);
		else {
			result = false;
			cout <<"Invalid flag!!! Flag format: rr rw wr ww.\t";
		}
		if (result)
			cout <<"There is an overlap between thread #"<<thread_id<<" and thread #"<<thread_id_2<<"!"<<endl;
		cout <<"Please enter the watchpoint you want to compare: <thread_id1> <thread_id2> <flag1(r/w)flag2(r/w)> "<<endl;
		cin >>thread_id >>thread_id_2 >>flag;
	}
	//	Manipulating Statistics
	cout <<"-----------------------Manipulating Statistics------------------------"<<endl;
	wp_test.print_statistics(*output);
	cout <<"Please enter the thread_id of the statistics you want to change: <thread_id1> += <thread_id2> "<<endl;
	cin >>thread_id >>thread_id_2;
	while (thread_id != -1) {
		wp_test.print_statistics(wp_test.get_statistics(thread_id), *output);
		cout <<" += "<<endl;
		wp_test.print_statistics(wp_test.get_statistics(thread_id_2), *output);
		wp_test.set_statistics(thread_id, wp_test.get_statistics(thread_id) + wp_test.get_statistics(thread_id_2));
		cout <<"result: "<<endl;
		wp_test.print_statistics(wp_test.get_statistics(thread_id), *output);
		cout <<"Please enter the thread_id of the statistics you want to change: <thread_id1> += <thread_id2> "<<endl;
		cin >>thread_id >>thread_id_2;
	}
	wp_test.print_statistics(*output);
	cout <<"Printing active only... "<<endl;
	wp_test.print_statistics(*output, true);
	return 0;
}
