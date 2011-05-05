
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
statistics_t& WatchPoint<ADDRESS, FLAGS>::operator +=(statistics_t &a, const statistics_t &b) {
	statistics_t result;
	result.checks = a.checks + b.checks;
	result.oracle_faults = a.oracle_faults + b.oracle_faults;
	result.sets = a.sets + b.sets;
	result.updates = a.updates + b.updates;
	return result;
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
	cout <<"Total number of faults checked: "<<to_print.faults<<endl;
	cout <<"Total number of \'set\'s: "<<to_print.sets<<endl;
	cout <<"Total number of \'update\'s: "<<to_print.updates<<endl;
	return;
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
