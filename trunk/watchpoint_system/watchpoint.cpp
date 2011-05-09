#ifndef WATCHPOINT_CPP_
#define WATCHPOINT_CPP_

#include "watchpoint.h"

/*
 *	operator that is used for a += b
 *		adding all statistics of b to a and then return the result to a
 */
inline statistics_t& operator +=(statistics_t &a, const statistics_t &b) {	//	not inline implementation will make it multiple definition
	a.checks += b.checks;
	a.oracle_faults += b.oracle_faults;
	a.sets += b.sets;
	a.updates += b.updates;
	return a;
}

/*
 *	operator that is used for c = a + b
 *		adding all statistics of a and b and then return the result
 */
inline statistics_t operator +(const statistics_t &a, const statistics_t &b) {
	statistics_t result;
	result.checks = a.checks + b.checks;
	result.oracle_faults = a.oracle_faults + b.oracle_faults;
	result.sets = a.sets + b.sets;
	result.updates = a.updates + b.updates;
	return result;
}

/*
 *	Constructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint() {
}

/*
 *	Destructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::~WatchPoint() {
}

/*
 *	Starting a thread with thread_id
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::start_thread(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter == statistics.end()) {							//	if thread_id is not active
		Oracle<ADDRESS, FLAGS> empty_oracle;
		/*
		 *	Initiating statistics
		 */
		statistics_inactive_iter = statistics_inactive.find(thread_id);	//	search for inactive thread_id
																		//	(just in case we start an inactive thread again, 
																		//		which is actually impossible)
		if (statistics_inactive_iter == statistics_inactive.end())		//	if not found
			statistics[thread_id] = clear_statistics();					//	initiate to zeros
		else {															//	if found
			statistics[thread_id] = statistics_inactive_iter->second;	//	copy the original inactive data
			statistics_inactive.erase(statistics_inactive_iter);		//	erase thread_id from inactive
		}
		/*
		 *	Initiating Oracle
		 */
		oracle_wp[thread_id] = empty_oracle;
		return 0;														//	normal start: return 0
	}
	return -1;															//	abnormal start: starting active thread again, return -1
}

/*
 *	Ending an active thread
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {						//	if thread_id is active
		oracle_wp_iter = oracle_wp.find(thread_id);
		statistics_inactive[thread_id] = statistics_iter->second;	//	move its statistics to inactive
		statistics.erase(statistics_iter);							//	remove it from active statistics
		oracle_wp.erase(oracle_wp_iter);							//	remove its Oracle watchpoint data
		return 0;													//	normal end: return 0
	}
	return -1;														//	abnormal end: thread_id not found or inactive, return -1
}

/*
 *	Printing all active thread_id's
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_threads() {
	cout <<"Printing active threads: "<<endl;
	for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {	//	for all active thread_id's
		cout <<"Thread #"<<statistics_iter->first<<" is active."<<endl;									//	print to std::cout
	}
	return;
}

/*
 *	reset all watchpoints and statistics (both active and inactive)
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset() {
	Oracle<ADDRESS, FLAGS> empty_oracle;
	statistics_t empty_statistics = clear_statistics();
	oracle_wp_iter = oracle_wp.begin();
	for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
																//	for all active thread_id's
		statistics_iter->second = empty_statistics;				//	clear statistics
		oracle_wp_iter->second = empty_oracle;					//	clear Oracle watchpoints
		oracle_wp_iter++;										//	Oracle should be of the same length as statistics
	}
	for (statistics_inactive_iter = statistics_inactive.begin();statistics_inactive_iter != statistics_inactive.end();statistics_inactive_iter++)
																//	for all inactive thread_id's
		statistics_inactive_iter->second = empty_statistics;	//	clear inactive statistics
	return;
}

/*
 *	reset all watchpoint and statistics (both active and inactive) of thread_id
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);		//	search for active thread_id
	if (statistics_iter != statistics.end()) {			//	if found active
		Oracle<ADDRESS, FLAGS> empty_oracle;
		oracle_wp_iter = oracle_wp.find(thread_id);
		statistics_iter->second = clear_statistics();	//	clear statistics
		oracle_wp_iter->second = empty_oracle;			//	clear Oracle watchpoints
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end())		//	if found inactive
			statistics_inactive_iter->second = clear_statistics();		//	clear inactive statistics
	}
	return;
}

/*
 *	Check for r/w faults in thead_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {									//	if thread_id found active
		bool oracle_fault = oracle_wp_iter->second.watch_fault(start, end);		//	check if Oracle fault
		if (!ignore_statistics) {												//	if not ignore statistics
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.checks++;									//	checks++
			if (oracle_fault)													//	and if it is a Oracle fault
				statistics_iter->second.oracle_faults++;						//	oracle_fault++
		}
		return oracle_fault;													//	return Oracle fault
	}
	return false;																//	return false if not found or inactive
}

/*
 *	Check for r faults in thead_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		bool oracle_fault = oracle_wp_iter->second.read_fault(start, end);	//	check for Oracle read fault
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.checks++;
			if (oracle_fault)
				statistics_iter->second.oracle_faults++;
		}
		return oracle_fault;
	}
	return false;
}

/*
 *	Check for w faults in thead_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		bool oracle_fault = oracle_wp_iter->second.write_fault(start, end);	//	check for Oracle write fault
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.checks++;
			if (oracle_fault)
				statistics_iter->second.oracle_faults++;
		}
		return oracle_fault;
	}
	return false;
}

/*
 *	Printing all existing watchpoints thread by thread (active only)
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_watchpoints() {
	cout <<"Printing all existing watchpoints: "<<endl;
	for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter != oracle_wp.end();oracle_wp_iter++) {
		cout <<"Watchpoints in thread #"<<oracle_wp_iter->first<<":"<<endl;
		oracle_wp_iter->second.watch_print();
	}
	cout <<"Watchpoint print ends."<<endl;
}

//set	 11	(r/w)
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {									//	if thread_id found
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ | WA_WRITE);	//	set	 11	(r/w)
		if (!ignore_statistics) {												//	if not ignored
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;										//	sets++
		}
		return 0;																//	normal set: return 0
	}
	return -1;																	//	abnormal set: thread_id not found, return -1
}

//set	 10
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ);				//	set r
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_WRITE);				//	reset w
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;										//	sets++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//set	 01
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_WRITE);			//	set w
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ);				//	reset r
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;										//	sets++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//set	 00
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ | WA_WRITE);	//	reset r/w
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.sets++;										//	sets++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//update 1x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_READ);				//	set r
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;									//	updates++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//update x1
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.add_watchpoint(start, end, WA_WRITE);			//	set w
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;									//	updates++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//update 0x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_READ);				//	reset r
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;									//	updates++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

//update x0
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
	oracle_wp_iter = oracle_wp.find(thread_id);
	if (oracle_wp_iter != oracle_wp.end()) {
		oracle_wp_iter->second.rm_watchpoint(start, end, WA_WRITE);				//	reset w
		if (!ignore_statistics) {
			statistics_iter = statistics.find(thread_id);
			statistics_iter->second.updates++;									//	updates++ (if !ignored)
		}
		return 0;
	}
	return -1;
}

/*
 *	Called by the other comparison functions
 */
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
			if (oracle_wp_iter->second.wp_iter->flags & flag_thread1) {
				/*
				 *	check for faults for required flags in thread #2
				 */
				if (oracle_wp_iter_2->second.general_fault(oracle_wp_iter->second.wp_iter->start_addr, 
														 oracle_wp_iter->second.wp_iter->end_addr, 
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

/*
 *	Get statistics for thread_id
 */
template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::get_statistics(int32_t thread_id) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {								//	if found active
		return statistics_iter->second;										//	return active statistics
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end()) {		//	if found inactive
			return statistics_inactive_iter->second;						//	return inactive statistics
		}
	}
	return clear_statistics();												//	if not found, return zero statistics
}

/*
 *	Set thread_id's statistics to input if found (both active and inactive)
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_statistics(int32_t thread_id, statistics_t input) {
	statistics_iter = statistics.find(thread_id);
	if (statistics_iter != statistics.end()) {							//	if found active
		statistics_iter->second = input;								//	set statistics = input
		return 0;														//	normal set: return 0
	}
	else {
		statistics_inactive_iter = statistics_inactive.find(thread_id);
		if (statistics_inactive_iter != statistics_inactive.end()) {	//	if found inactive
			statistics_inactive_iter->second = input;					//	set statistics = input
			return 0;													//	normal set: return 0
		}
	}
	return -1;															//	abnormal set: tread_id not found, return -1
}

/*
 *	Returns a zero statistics_t
 */
template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::clear_statistics() {
	statistics_t empty;
	empty.checks=0;
	empty.oracle_faults=0;
	empty.sets=0;
	empty.updates=0;
	return empty;
}

/*
 *	Print statistics thread by thread
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(bool active) {
	if (active) {		//	if active, only print active threads' statistics only
		cout <<"Printing statistics for all active threads: "<<endl;
		for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
			cout <<"Statistics of thead #"<<statistics_iter->first<<" : "<<endl;
			print_statistics(statistics_iter->second);
		}
	}
	else {				//	if !active, print all threads' statistics
		cout <<"Printing statistics for all threads (both active and inactive): "<<endl;
		statistics_iter = statistics.begin();
		statistics_inactive_iter = statistics_inactive.begin();
		while (statistics_iter != statistics.end() || statistics_inactive_iter != statistics_inactive.end()) {
			if (statistics_inactive_iter == statistics_inactive.end()) {
				cout <<"Statistics of active thead #"<<statistics_iter->first<<" : "<<endl;
				print_statistics(statistics_iter->second);
				statistics_iter++;
			}
			else if (statistics_iter == statistics.end()) {
				cout <<"Statistics of inactive thead #"<<statistics_inactive_iter->first<<" : "<<endl;
				print_statistics(statistics_inactive_iter->second);
				statistics_inactive_iter++;
			}
			else {
				if (statistics_iter->first <= statistics_inactive_iter->first) {
					cout <<"Statistics of active thead #"<<statistics_iter->first<<" : "<<endl;
					print_statistics(statistics_iter->second);
					statistics_iter++;
				}
				else {
					cout <<"Statistics of inactive thead #"<<statistics_inactive_iter->first<<" : "<<endl;
					print_statistics(statistics_inactive_iter->second);
					statistics_inactive_iter++;
				}
			}
		}
	}
	cout <<"Statistics printing ends."<<endl;
	return;
}

/*
 *	Print statistics (called by print_statistics(bool active) )
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(const statistics_t& to_print) {
	cout <<"Total number of checks for faults: "<<to_print.checks<<endl;
	cout <<"Total number of faults checked: "<<to_print.oracle_faults<<endl;
	cout <<"Total number of \'set\'s: "<<to_print.sets<<endl;
	cout <<"Total number of \'update\'s: "<<to_print.updates<<endl;
	cout <<endl;
	return;
}

#endif /* WATCHPOINT_CPP_ */
