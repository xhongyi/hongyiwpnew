#ifndef RANGE_CACHE_CPP_
#define RANGE_CACHE_CPP_

#include "range_cache.h"
#include <iostream>

using namespace std;

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::RangeCache() {
}

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::~RangeCache() {
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::search(ADDRESS target_addr, bool update) {
   // binary search 
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator beg_iter;
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator mid_iter;
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator end_iter;
	beg_iter = wp.begin();
	end_iter = wp.end();
	while (beg_iter < end_iter - 1 ) {
		mid_iter = beg_iter + (end_iter - beg_iter) / 2;
		if (target_addr <= mid_iter->start_addr)
			end_iter = mid_iter;
		else
			beg_iter = mid_iter;
	}
	if (target_addr <= beg_iter->end_addr) {  // check if beg_iter
	   lru.search(beg_iter->start_addr, beg_iter->end_addr, update);
	   return beg_iter;
	}
	if (end_iter != wp.end() && target_addr >= end_iter->start_addr && target_addr <= end_iter->end_addr) {
	                                          // check if end_iter
	   lru.search(end_iter->start_addr, end_iter->end_addr, update);
	   return end_iter;
	}
	return wp.end();
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::search_next(ADDRESS target_addr, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::add_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::remove_range(ADDRESS start_addr, ADDRESS end_addr, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::overwrite_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::add_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::remove_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
}

template<class ADDRESS, class FLAGS>
bool RangeCache<ADDRESS, FLAGS>::cache_overflow() {
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::cache_kickout() {
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::search_address(ADDRESS target_addr) {
}

#endif
