#ifndef WATCHPOINT_CPP_
#define WATCHPOINT_CPP_

#include "watchpoint.h"

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
		Oracle empty_oracle;
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter == statistics_inactive.end())
			statistics[tread_id] = clear_statistics();
		else {
			statistics[tread_id] = statistics_inactive_iter->second;
			statistics_inactive.erase(statistics_inactive_iter);
		}
		oracle_wp[tread_id] = empty_oracle;
		return 0;
	}
	else {
		return -1;
	}
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter == statistics.end()) {
		return -1;
	}
	else {
		oracle_wp_iter = oracle_wp.find(thread_id);
		statistics_inactive[thread_id] = statistics_iter->second;
		statistics.erase(statistics_iter);
		oracle_wp.erase(oracle_wp_iter);
		return 0;
	}
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
	Oracle empty_oracle;
	statistics_t empty_statistics = clears_statistics();
	oracle_wp_iter = oracle_wp.begin();
	for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
		statistics_iter->second = empty_statistics;
		oracle_wp_iter->second = empty_oracle;
	}
	for (statistics_inactive_iter = statistics_inactive.begin();statistics_inactive_iter != statistics_inactive.end();statistics_inactive_iter++)
		statistics_inactive_iter->second = empty_statistics;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {
		Oracle empty_oracle;
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
bool  WatchPoint<ADDRESS, FLAGS>::watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false) {
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
bool  WatchPoint<ADDRESS, FLAGS>::read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false) {
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
bool  WatchPoint<ADDRESS, FLAGS>::write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false) {
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
int WatchPoint<ADDRESS, FLAGS>::set_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	oracle_wp_iter->second.add_watchpoint(start, end, WA_READ | WA_WRITE);
	if (!ignore_statistics) {
		statistics_iter = statistics.find(thread_id);
		statistics_iter->second.update++;
	}
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

#endif /* WATCHPOINT_CPP_ */
