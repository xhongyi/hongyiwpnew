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
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator 
	   search_iter = search_address(target_addr);
	if (search_iter == wp.end() || target_addr < search_iter->start_addr)
	   return wp.end();
	else
	   lru.search(search_iter->start_addr, search_iter->end_addr, update)
	   return search_iter;
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::search_next(ADDRESS target_addr, bool update) {
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::add_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
   // construct a new watchpoint
   watchpoint_t<ADDRESS, FLAGS> temp;
   temp.start_addr = start_addr;
   temp.end_addr = end_addr;
   temp.flags = target_flags;
   // search for a proper location
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator 
      add_iter = search_address(start_addr);
   // add to range cache
   add_iter = wp.insert(add_iter, temp);
   // update lru (do NOT need to check bool update ???)
   lru.insert(start_addr, end_addr);
   // check for merge
   if (add_iter != wp.begin()) {
      add_iter--;
      if ( add_iter->end_addr == start_addr-1 && add_iter->flags == target_flags ) {
         lru.remove(add_iter->start_addr, add_iter->end_addr);
         lru.modify(start_addr, end_addr, add_iter->start_addr, end_addr, update);
         start_addr = add_iter->start_addr;
         add_iter->end_addr = end_addr;
         add_iter = wp.erase(add_iter+1);
      }
      else {
         add_iter += 2;
      }
   }
   if (add_iter != wp.end()) {
      if ( add_iter->start_addr == end_addr+1 && add_iter->flags == target_flags ) {
         lru.remove(add_iter->start_addr, add_iter->end_addr);
         lru.modify(start_addr, end_addr, start_addr, add_iter->end_addr, update);
         add_iter->start_addr = start_addr;
         wp.erase(add_iter-1);
      }
   }
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::remove_range(ADDRESS start_addr, ADDRESS end_addr, bool update) {
   // search for a start location
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator 
      rm_iter = search_address(start_addr);
   // check start
   if (rm_iter->start_addr < start_addr) {
      lru.modify(rm_iter->start_addr, rm_iter->end_addr, rm_iter->start_addr, start_addr-1, update);
      if (rm_iter->end_addr > end_addr) {    // check split
         lru.insert(end_addr+1, rm_iter->end_addr);
         // construct a new watchpoint
         watchpoint_t<ADDRESS, FLAGS> temp;
         temp.start_addr = end_addr+1;
         temp.end_addr = rm_iter->end_addr;
         temp.flags = rm_iter->flags;
         rm_iter->end_addr = start_addr-1;
         rm_iter = wp.insert(rm_iter+1, temp);
      }
      else {
         rm_iter->end_addr = start_addr-1;
         rm_iter++;
      }
   }
   // remove all ranges inside
   while (rm_iter != wp.end() && rm_iter->end_addr <= end_addr) {
      lru.remove(rm_iter->start_addr, rm_iter->end_addr);
      rm_iter = wp.erase(rm_iter);
   }
   // check end
   if (rm_iter != wp.end() && rm_iter->start_addr <= end_addr) {
      lru.modify(rm_iter->start_addr, rm_iter->end_addr, end_addr+1, rm_iter->end_addr, update);
      rm_iter->start_addr = end_addr+1;
   }
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::overwrite_range(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
   // clear watchpoints within the range
   remove_range(start_addr, end_addr, update);
   // overwrite new watchpoint range
   add_range(start_addr, end_addr, update);
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::flag_operation(ADDRESS start_addr, ADDRESS end_addr,
		FLAGS (*flag_op)(FLAGS &x, FLAGS &y), FLAGS target_flags, bool update) {
   // construct a new watchpoint
   watchpoint_t<ADDRESS, FLAGS> temp;
   // search for a start location
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator 
      modify_iter = search_address(start_addr);
   // check start
   if (modify_iter->start_addr < start_addr) {
      lru.modify(modify_iter->start_addr, modify_iter->end_addr, modify_iter->start_addr, start_addr-1, update);
      lru.insert(start_addr, modify_iter->end_addr);
      temp.start_addr = start_addr;
      temp.end_addr = modify_iter->end_addr;
      temp.flags = modify_iter->flags;
      // split from start, do NOT modify flags
      modify_iter->end_addr = start_addr-1;
      modify_iter = wp.insert(modify_iter+1, temp);
      /*if (modify_iter->end_addr > end_addr) {    // check split at end_addr
         temp.start_addr = end_addr+1;
         temp.end_addr = modify_iter->end_addr;
         temp.flags = modify_iter->flags;
         // split from start
         modify_iter->end_addr = start_addr-1;
         // split from end
         lru.insert(end_addr+1, modify_iter->end_addr);
         modify_iter = wp.insert(modify_iter+1, temp);
         // modify flag
         lru.insert(start_addr, end_addr);
         temp.start_addr = start_addr;
         temp.end_addr = end_addr;
         temp.flags = flag_op(modify_iter->flags, target_flags);
         modify_iter = wp.insert(modify_iter, temp);
         modify_iter++;
      }
      else {
         lru.insert(start_addr, modify_iter->end_addr);
         temp.start_addr = start_addr;
         temp.end_addr = modify_iter->end_addr;
         temp.flags = flag_op(modify_iter->flags, target_flags);
         // split from start
         modify_iter->end_addr = start_addr-1;
         // modify flag
         modify_iter = wp.insert(modify_iter+1, temp);
         modify_iter++;
      }*/
   }
   // modify all ranges inside
   while (modify_iter != wp.end() && modify_iter->end_addr <= end_addr) {
      lru.modify(modify_iter->start_addr, modify_iter->end_addr, modify_iter->start_addr, modify_iter->end_addr, update);
      modify_iter->flags = flag_op(modify_iter->flags, target_flags);
      modify_iter++;
   }
   // check end
   if (modify_iter != wp.end() && modify_iter->start_addr <= end_addr) {
      lru.modify(modify_iter->start_addr, modify_iter->end_addr, end_addr+1, modify_iter->end_addr, update);
      lru.insert(modify_iter->start_addr, end_addr);
      temp.start_addr = modify_iter->start_addr;
      temp.end_addr = end_addr;
      temp.flags = flag_op(modify_iter->flags, target_flags);
      modify_iter->start_addr = end_addr+1;
      modify_iter = wp.insert(modify_iter, temp);
   }
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::add_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
   flag_operation(start_addr, end_addr, &flag_union, target_flags, update);
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::remove_flag(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool update) {
   flag_operation(start_addr, end_addr, &flag_diff, target_flags, update);
}

template<class ADDRESS, class FLAGS>
bool RangeCache<ADDRESS, FLAGS>::cache_overflow() {
   if (wp.size() > CACHE_SIZE)
      return true;
   return false;
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::cache_kickout() {
   Range<ADDRESS> temp = lru.kickout();
   return wp.erase(search_address(temp.start_addr));
}

template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
RangeCache<ADDRESS, FLAGS>::search_address(ADDRESS target_addr) {
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
	if (target_addr <= beg_iter->end_addr)    // check if beg_iter
	   return beg_iter;
	else if (end_iter == wp.end() || target_addr <= end_iter->end_addr)
	                                          // check if end_iter
	   return end_iter;
	else
	   return end_iter+1;
}

#endif
