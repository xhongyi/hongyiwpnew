#ifndef WATCHPOINT_CPP_
#define WATCHPOINT_CPP_

#include "watchpoint.h"

statistics_t& operator +=(statistics_t &a, const statistics_t &b) {
	a.checks += b.checks;
	a.oracle_faults += b.oracle_faults;
	a.sets += b.sets;
	a.updates += b.updates;
	return a;
}

template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint() {
}

template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::~WatchPoint() {
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::start_thread(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter == statistics.end()) {
		Oracle<ADDRESS, FLAGS> empty_oracle;
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter == statistics_inactive.end())
			statistics[thread_id] = clear_statistics();
		else {
			statistics[thread_id] = statistics_inactive_iter->second;
			statistics_inactive.erase(statistics_inactive_iter);
		}
		oracle_wp[thread_id] = empty_oracle;
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {
		oracle_wp_iter = oracle_wp.find(thread_id);
		statistics_inactive[thread_id] = statistics_iter->second;
		statistics.erase(statistics_iter);
		oracle_wp.erase(oracle_wp_iter);
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_threads() {
	cout <<"Printing active threads: "<<endl;
	for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
		cout <<"Thread #"<<statistics_iter->first<<" is active."<<endl;
	}
	return 0;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset() {
	Oracle<ADDRESS, FLAGS> empty_oracle;
	statistics_t empty_statistics = clear_statistics();
	oracle_wp_iter = oracle_wp.begin();
	for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
		statistics_iter->second = empty_statistics;
		oracle_wp_iter->second = empty_oracle;
	}
	for (statistics_inactive_iter = statistics_inactive.begin();statistics_inactive_iter != statistics_inactive.end();statistics_inactive_iter++)
		statistics_inactive_iter->second = empty_statistics;
	return;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {
		Oracle<ADDRESS, FLAGS> empty_oracle;
		oracle_wp_iter = oracle_wp.find(thread_id);
		statistics_iter->second = clear_statistics();
		oracle_wp_iter->second = empty_oracle;
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end())
			statistics_iter->second = clear_statistics();
	}
	return;
}

template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	bool oracle_fault = oracle_wp[thread_id].watch_fault(start, end);
	if (!ignore_statistics) {
		statistics_iter = statistics.find(thread_id);
		statistics_iter->second.checks++;
		if (oracle_fault)
			statistics_iter->second.oracle_faults++;
	}
	return oracle_fault;
}

template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	bool oracle_fault = oracle_wp[thread_id].read_fault(start, end);
	if (!ignore_statistics) {
		statistics_iter = statistics.find(thread_id);
		statistics_iter->second.checks++;
		if (oracle_fault)
			statistics_iter->second.oracle_faults++;
	}
	return oracle_fault;
}

template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	bool oracle_fault = oracle_wp[thread_id].write_fault(start, end);
	if (!ignore_statistics) {
		statistics_iter = statistics.find(thread_id);
		statistics_iter->second.checks++;
		if (oracle_fault)
			statistics_iter->second.oracle_faults++;
	}
	return oracle_fault;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_watchpoints() {
	cout <<"Printing all existing watchpoints: "<<endl;
	for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter != oracle_wp.end();oracle_wp_iter++) {
		cout <<"Watchpoints in thread #"<<oracle_wp_iter->first<<":"<<endl;
		oracle_wp_iter->second.watch_print();
	}
	cout <<"Watchpoint print ends."<<endl;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ | WA_WRITE);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ);
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_WRITE);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_WRITE);
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ | WA_WRITE);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_WRITE);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_WRITE);
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;
		}
		return 0;
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::general_compare(int32_t thread_id1, int32_t thread_id2, FLAGS flag_thread1, FLAGS flag_thread2) {
	/*
	 *	search the two threads in oracle_wp
	 */
	oracle_wp_iter = oracle_wp.find(thread_id1);
	oracle_wp_iter_2 = oracle_wp.find(thread_id2);
	/*
	 *	if both are found in oracle_wp
	 */
	if (oracle_wp_iter != oracle_wp.end() && oracle_wp_iter_2 != oracle_wp.end() ) {
		/*
		 *	for each watchpoint ranges in thread #1
		 */
		for (oracle_wp_iter->second.wp_iter = oracle_wp_iter->second.wp.begin();
				oracle_wp_iter->second.wp_iter != oracle_wp_iter->second.wp.end();
				oracle_wp_iter->second.wp_iter++) {
			/*
			 *	check only for required flags in thread #1
			 */
			if (oracle_wp_iter->second.wp_iter->second.flags & flag_thread1) {
				/*
				 *	check for faults for required flags in thread #2
				 */
				if (oracle_wp_iter->second.general_fault(oracle_wp_iter->second.wp_iter->second.start_addr, 
																					oracle_wp_iter->second.wp_iter->second.end_addr, 
																					flag_thread2) ) {
					return true;
				}
			}
		}
	}
	return false;
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_rr(int32_t thread_id1, int32_t thread_id2) {
	return general_compare(thread_id1, thread_id2, WA_READ, WA_READ);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_rw(int32_t thread_id1, int32_t thread_id2) {
	return general_compare(thread_id1, thread_id2, WA_READ, WA_WRITE);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_wr(int32_t thread_id1, int32_t thread_id2) {
	return general_compare(thread_id1, thread_id2, WA_WRITE, WA_READ);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_ww(int32_t thread_id1, int32_t thread_id2) {
	return general_compare(thread_id1, thread_id2, WA_WRITE, WA_WRITE);
}

template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::get_statistics(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {
		return statistics_iter->second;
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end()) {
			return statistics_inactive_iter->second;
		}
	}
	return clear_statistics();
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_statistics(int32_t thread_id, statistics_t input) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {
		statistics_iter->second = input;
		return 0;
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end()) {
			statistics_inactive_iter->second = input;
			return 0;
		}
	}
	return -1;
}

template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::clear_statistics() {
	statistics_t empty;
	empty.checks=0;
	empty.oracle_faults=0;
	empty.sets=0;
	empty.updates=0;
	return empty;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(bool active) {
	if (active) {
		cout <<"Printing statistics for all active threads: "<<endl;
		for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
			cout <<"Statistics of thead #"<<statistics_iter->first<<" : "<<endl;
			print_statistics(statistics_iter->second);
		}
	}
	else {
		cout <<"Printing statistics for all threads (both active and inactive): "<<endl;
		statistics_iter = statistics.begin();
		statistics_inactive_iter = statistics_inactive.begin();
		while (statistics_iter != statistics.end() && statistics_inactive_iter != statistics_inactive.end()) {
			if (statistics_inactive_iter == statistics_inactive.end()) {
				cout <<"Statistics of thead #"<<statistics_iter->first<<" : "<<endl;
				print_statistics(statistics_iter->second);
				statistics_iter++;
			}
			else if (statistics_iter == statistics.end()) {
				cout <<"Statistics of thead #"<<statistics_inactive_iter->first<<" : "<<endl;
				print_statistics(statistics_inactive_iter->second);
				statistics_inactive_iter++;
			}
			else {
				if (statistics_iter->first <= statistics_inactive_iter->first) {
					cout <<"Statistics of thead #"<<statistics_iter->first<<" : "<<endl;
					print_statistics(statistics_iter->second);
					statistics_iter++;
				}
				else {
					cout <<"Statistics of thead #"<<statistics_inactive_iter->first<<" : "<<endl;
					print_statistics(statistics_inactive_iter->second);
					statistics_inactive_iter++;
				}
			}
		}
	}
	cout <<"Statistics printing ends."<<endl;
	return;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(statistics_t& to_print) {
	cout <<"Total number of checks for faults: "<<to_print.checks<<endl;
	cout <<"Total number of faults checked: "<<to_print.oracle_faults<<endl;
	cout <<"Total number of \'set\'s: "<<to_print.sets<<endl;
	cout <<"Total number of \'update\'s: "<<to_print.updates<<endl;
	return;
}

#endif /* WATCHPOINT_CPP_ */
