#include "../watchpoint_system/watchpoint.h"
#include <iostream>
using namespace std;

int main() {
	WatchPoint<unsigned int, unsigned int> wp_test;
	for (int i=0;i<5;i++) {
		if (wp_test.start_thread(i))
			cout <<"Failed to start thread #"<<i<<" !!!"<<endl;
	}
	if (!wp_test.start_thread(2))
		cout <<"Starting the same thread again!!!"<<endl;
	wp_test.print_watchpoints();
	for (int i=0;i<5;i++) {
		if (wp_test.end_thread(i))
			cout <<"Failed to end thread #"<<i<<" !!!"<<endl;
	}
	return 0;
}
