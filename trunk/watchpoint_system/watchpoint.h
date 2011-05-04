/*
 *	watchpoint.h
 *
 *	Created on: May 4, 2011
 *		Author: luoyixin
 */

#ifndef WATCHPOINT_H_
#define WATCHPOINT_H_

#include "oracle_wp.h"
#include <map>

struct statistics_t {
	long long checks;
	long long faults;
	long long sets;
	long long updates;
};

template<class ADDRESS, class FLAGS>
class WatchPoint {
public:
	Watchpoint();
	~Watchpoint();
	
	int start_thread(int32_t thread_id);
	int end_thread(int32_t thread_id);
	void print_threads();
	
	void reset();int32_t
	void reset(int32_t thread_id);
	
	bool watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	bool read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	bool write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);
	void print_watchpoints();
	
	int set_watch		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 11
	int set_read		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set x1
	int set_write		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set 1x
	int update_set_read	(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 01
	int update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 10
	int rm_watch		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//update 00
	int rm_read			(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set x0
	int rm_write		(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);	//set 0x
	
	bool compare_rr(int32_t thread_id1, int32_t thread_id2);
	bool compare_rw(int32_t thread_id1, int32_t thread_id2);
	bool compare_wr(int32_t thread_id1, int32_t thread_id2);
	bool compare_ww(int32_t thread_id1, int32_t thread_id2);
	
	statistics_t get_statistics(int32_t thread_id);
	int set_statistics(int32_t thread_id, statistics_t input);
	statistics_t clear_statistics();
	statistics_t operator+(const statistics_t other);
	void print_statistics(bool active);
	void print_statistics(statistics_t& to_print);

private:
	map<int32_t, Oracle> oracle_wp;
	map<int32_t, statistics_t> statistics;
	
	map<int32_t, statistics_t>::iterator statistics_iter;
	map<int32_t, Oracle>::iterator oracle_wp_iter;
};

#include "watchpoint.cpp"
#endif /* WATCHPOINT_H_ */
