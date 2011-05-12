/*
 * page_table_wp.h
 *
 *  Created on: May 1, 2011
 *      Author: luoyixin
 */

#ifndef PAGE_TABLE_WP_H_
#define PAGE_TABLE_WP_H_

#define  VIRTUAL_ADDRESS_LENGTH  32
#define  PAGE_OFFSET_LENGTH      12       //page size is 4 KB
#define  PAGE_NUMBER             (1<<12)
#define  BIT_MAP_OFFSET_LENGTH   5        //we store one bit for each page in a bit map, 
                                          //which is byte addressable
#define  BIT_MAP_NUMBER_LENGTH   (VIRTUAL_ADDRESS_LENGTH - PAGE_OFFSET_LENGTH - BIT_MAP_OFFSET_LENGTH)
#define  BIT_MAP_NUMBER          (1<<BIT_MAP_NUMBER_LENGTH) //needs 128 KB to store these bits for each thread

#define  MAX_THREADS             (1<<BIT_MAP_OFFSET_LENGTH)

#include <stdint.h>     //contains int32_t
#include <map>
#include <queue>
#include "oracle_wp.h"

template<class ADDRESS, class FLAGS>
class WatchPoint_PT {
public:
	WatchPoint_PT();
	~WatchPoint_PT();
	
	void     start_thread   (int32_t thread_id, Oracle<ADDRESS, FLAGS>* oracle_ptr_in);
	void     end_thread     (int32_t thread_id);
	
	/*
	 * this function tells all pages covered by this range is watched or not
	 */
	bool  watch_fault    (ADDRESS start_addr, ADDRESS end_addr);
	
	/*
	 * we don't need these functions because the page table do not keep 
	 * what kind of watchpoint it is having
	 */
	
	//bool   read_fault  (ADDRESS start_addr, ADDRESS end_addr);
	//bool   write_fault (ADDRESS start_addr, ADDRESS end_addr);
	
	//void     watch_print (ostream &output = cout);
	
	/*
	 * returns the number of changes it does on bit_map
	 * for set watchpoint: just call add_watchpoint
	 * for update watchpoint: needs to call rm_watchpoint on all ranges and then add_watchpoint to overwrite
	 * for counting the number of changes: update = abs(add - rm);
	 *
	 * add_watchpoint needs target_flags just to check if it is none flag or not
	 */
	int   add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, int32_t thread_id, FLAGS target_flags);
	int   rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, int32_t thread_id);
	
	void set_bit_map(int32_t thread_id, ADDRESS page_number);
	void reset_bit_map(int32_t thread_id, ADDRESS page_number);
	
	void get_bit(ADDRESS page_number);
	void get_bit(ADDRESS page_number, int32_t thread_id);

//private:
	/*
	 * maps used to store: 1. pointers to oracles 2. bit_map sequence numbers  for each thread
	 */
	map<int32_t, Oracle<ADDRESS, FLAGS>* >                      oracle_wp_ptr;
	map<int32_t, unsigned int>                                  bit_map_num;
	queue<unsigned int>                                         avail_bit_map_num;
	
	typename map<int32_t, Oracle<ADDRESS, FLAGS>* >::iterator   oracle_wp_ptr_iter;
	map<int32_t, unsigned int>::iterator                        bit_map_num_iter;
	
	unsigned int log_max_threads;
	
	/*
	 * one bit for each page each thread
	 * keeping track of whether this page is watched or not
	 * the software will keep what kind of watchpoint it is
	 */
	unsigned int *bit_map;
};

#include "page_table_wp.cpp"
#endif /* PAGE_TABLE_WP_H_ */
