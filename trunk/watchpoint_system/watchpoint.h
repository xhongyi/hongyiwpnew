/*
 *	watchpoint.h
 *
 *	Created on: May 4, 2011
 *		Author: luoyixin
 */

#ifndef WATCHPOINT_H_
#define WATCHPOINT_H_

#include <stdint.h>
#include <map>
#include "oracle_wp.h"

struct statistics_t {
	long long checks;
	long long oracle_faults;
	long long sets;
	long long updates;
};

statistics_t& operator +=(statistics_t &a, const statistics_t &b);

template<class ADDRESS, class FLAGS>
class WatchPoint {
public:
	WatchPoint();
	~WatchPoint();
	
	int start_thread(int32_t thread_id);
	int end_thread(int32_t thread_id);
	void print_threads();
	
	void reset();
	void reset(int32_t thread_id);
	
	bool watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	bool read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	bool write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	void print_watchpoints();
	
	int set_watch		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set	 11 (rw)
	int set_read		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set	 10
	int set_write		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set	 01
	int rm_watch		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set	 00
	
	int update_set_read	(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 1x
	int update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update x1
	int rm_read			(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 0x
	int rm_write		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update x0
	
	bool general_compare(int32_t thread_id1, int32_t thread_id2, FLAGS flag_thread1, FLAGS flag_thread2);
	bool compare_rr(int32_t thread_id1, int32_t thread_id2);
	bool compare_rw(int32_t thread_id1, int32_t thread_id2);
	bool compare_wr(int32_t thread_id1, int32_t thread_id2);
	bool compare_ww(int32_t thread_id1, int32_t thread_id2);
	
	statistics_t get_statistics(int32_t thread_id);
	int set_statistics(int32_t thread_id, statistics_t input);
	statistics_t clear_statistics();
	void print_statistics(bool active);
	void print_statistics(statistics_t& to_print);

private:
	map<int32_t, Oracle<ADDRESS, FLAGS> > oracle_wp;
	map<int32_t, statistics_t> statistics;
	map<int32_t, statistics_t> statistics_inactive;
	
	typename map<int32_t, Oracle<ADDRESS, FLAGS> >::iterator oracle_wp_iter;
	typename map<int32_t, Oracle<ADDRESS, FLAGS> >::iterator oracle_wp_iter_2;
	map<int32_t, statistics_t>::iterator statistics_iter;
	map<int32_t, statistics_t>::iterator statistics_inactive_iter;
};

#include "watchpoint.cpp"
#endif /* WATCHPOINT_H_ */
