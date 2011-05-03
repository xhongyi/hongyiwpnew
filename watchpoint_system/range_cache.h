#ifndef RANGE_CACHE_H_
#define RANGE_CACHE_H_
#include <deque>
#include "wp_data_struct.h"

template<class ADDRESS, class FLAGS>
class RangeCache {
public:
	RangeCache();
	~RangeCache();
	
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
		search(ADDRESS target_addr);
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator
		search_update(ADDRESS target_addr);
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
		search_next(ADDRESS target_addr);
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator
		search_next_update(ADDRESS target_addr);
	
	void add_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void add_range_update(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void remove_range(ADDRESS start_addr, ADDRESS end_addr);
	void remove_range_update(ADDRESS start_addr, ADDRESS end_addr);
	void overwrite_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void overwrite_range_update(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	
	void add_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void add_flag_update(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void remove_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void remove_flag_update(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	
	bool cache_overflow();
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator
		cache_kickout();
	 
//private:
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
		search_address(ADDRESS target_addr);
/*
	void	wp_operation	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags,
			 bool (*flag_test)(FLAGS &x, FLAGS &y), FLAGS (*flag_op)(FLAGS &x, FLAGS &y),
			 bool add_trie);
	void	add_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void	rm_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	bool	general_fault	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
*/
	/*
	 * wp		is the container to hold all the ranges for watchpoint structure.
	 * wp_iter	is the iterator used in both add_watchpoint and rm_watchpoint.
	 * pre_iter	is used for getting the node right before wp_iter.
	 * aft_iter	is used for getting the node right after wp_iter.
	 *
	 * All these data are declared here is to save time by avoiding creating
	 * and destroying them every time add_watchpoint and rm_watchpoint is called.
	 */
	std::deque< watchpoint_t<ADDRESS, FLAGS> > wp;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator wp_iter;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator pre_iter;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator aft_iter;
};

#include "range_cache.cpp"
#endif /*RANGE_CACHE_H_*/
