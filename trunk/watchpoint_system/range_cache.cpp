#ifndef RANGE_CACHE_CPP_
#define RANGE_CACHE_CPP_

#include "range_cache.h"
#include <iostream>

using namespace std;

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::RangeCache(Oracle<ADDRESS, FLAGS> *wp_ref) {
   oracle_wp = wp_ref;
   kickout_dirty=0;
   kickout=0;
   complex_updates=0;
}

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::RangeCache() {
   kickout_dirty=0;
   kickout=0;
   complex_updates=0;
}

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::~RangeCache() {
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, bool is_update) {
   return wp_operation(start_addr, end_addr, is_update);
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, bool is_update) {
   return wp_operation(start_addr, end_addr, is_update);
}
// we only need to know if it is a hit or miss in a range cache
//    so it is regardless of the checked flags
template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr) {
   int rc_miss = 0;
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_read_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   bool searching = true;     // searching = true until all ranges are covered
   while (searching) {
      rc_read_iter = search_address(start_addr);               // search starts from the start_addr
      if (rc_read_iter == rc_data.end()) {                     // if cache miss
         rc_miss++;                                            //    miss++
         // get new range from backing store
         rc_read_iter = oracle_wp->search_address(start_addr);
         rm_range(rc_read_iter->start_addr, rc_read_iter->end_addr);
         rc_data.push_front(*rc_read_iter);                    // push the new range to range cache
         rc_read_iter = search_address(start_addr);
      }
      if (rc_read_iter->end_addr >= end_addr)                  // if all ranges are covered
         searching = false;
      // refresh start_addr
      temp = *rc_read_iter;
      start_addr = temp.end_addr+1;
      // refresh lru
      rc_data.erase(rc_read_iter);                             // refresh this entry as most recently used
      rc_data.push_front(temp);
   }
   while (cache_overflow())                                    // kick out redundant cache entries
      cache_kickout();
   return rc_miss;
}
// wp_operation is same for add or rm a watchpoint, 
//    both simulated by removing all covered ranges and then getting new ranges from backing store
template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr, bool is_update) {
   int rc_miss = 0;
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_write_iter, oracle_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   if (is_update) {
      // support for counting complex updates
      bool searching = true;     // searching = true until all ranges are covered
      while (searching) {
         rc_write_iter = search_address(start_addr);               // search starts from the start_addr
         oracle_iter = oracle_wp->search_address(start_addr);
         if (rc_write_iter != rc_data.end()) {                     // if cache hit
            temp = *rc_write_iter;
            temp.flags = oracle_iter->flags | DIRTY;
            if (temp.end_addr >= end_addr)
               searching = false;
            else
               start_addr = temp.end_addr+1;
            rc_data.erase(rc_write_iter);
            rc_data.push_front(temp);
         }
         else {                                                   // if cache miss
            rc_miss++;                                            //    miss++
            rm_range(oracle_iter->start_addr, oracle_iter->end_addr);
            temp = *oracle_iter;
            temp.flags |= DIRTY;
            if (temp.end_addr >= end_addr)
               searching = false;
            else
               start_addr = temp.end_addr+1;
            rc_data.push_front(temp);                     // push the new range to range cache
         }
      }
   }
   else {
      rm_range(start_addr, end_addr);
      oracle_iter = oracle_wp->search_address(start_addr);
      temp = *oracle_iter;
      if (oracle_iter->start_addr < start_addr) {
         rc_write_iter = search_address(start_addr-1);
         temp.start_addr = rc_write_iter->start_addr;
         rc_data.erase(rc_write_iter);
      }
      if (oracle_iter->end_addr < end_addr) {
         rc_write_iter = search_address(end_addr+1);
         temp.end_addr = rc_write_iter->end_addr;
         rc_data.erase(rc_write_iter);
      }
      temp.flags |= DIRTY;
      rc_data.push_front(temp);
   }
   while (cache_overflow())                                    // kick out redundant cache entries
      cache_kickout();
   return rc_miss;
}

template<class ADDRESS, class FLAGS>
bool RangeCache<ADDRESS, FLAGS>::cache_overflow() {
   if (rc_data.size() > CACHE_SIZE)
      return true;
   return false;
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::cache_kickout() {
   kickout++;
   if (rc_data.back().flags & DIRTY)
      kickout_dirty++;
   rc_data.pop_back();
}
// perform a linear search in the range cache from the most recent used entry
//    return end if not found
template<class ADDRESS, class FLAGS>
typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator 
 RangeCache<ADDRESS, FLAGS>::search_address(ADDRESS target_addr) {
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator i;
   for (i=rc_data.begin();i!=rc_data.end();i++) {
      if (target_addr >= i->start_addr && target_addr <= i->end_addr)
         return i;
   }
   return rc_data.end();
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::rm_range(ADDRESS start_addr, ADDRESS end_addr) {
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_rm_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   rc_rm_iter = search_address(start_addr);
   // split start if needed
   if (rc_rm_iter!=rc_data.end() && rc_rm_iter->start_addr < start_addr) {
      temp.start_addr = rc_rm_iter->start_addr;
      temp.end_addr = start_addr-1;
      temp.flags = rc_rm_iter->flags;
      rc_rm_iter->start_addr = start_addr;
      rc_data.push_front(temp);
   }
   rc_rm_iter = search_address(end_addr);
   // split end if needed
   if (rc_rm_iter!=rc_data.end() && rc_rm_iter->end_addr > end_addr) {
      temp.start_addr = end_addr+1;
      temp.end_addr = rc_rm_iter->end_addr;
      temp.flags = rc_rm_iter->flags;
      rc_rm_iter->end_addr = end_addr;
      rc_data.push_front(temp);
   }
   unsigned int i=0;
   while (i < rc_data.size()) {
      rc_rm_iter = rc_data.begin()+i;
      if (rc_rm_iter->start_addr >= start_addr && rc_rm_iter->start_addr <= end_addr && 
          rc_rm_iter->end_addr   >= start_addr && rc_rm_iter->end_addr   <= end_addr)
         rc_data.erase(rc_rm_iter);
      else
         i++;
   }
}

template<class ADDRESS, class FLAGS>
void  RangeCache<ADDRESS, FLAGS>::
get_stats(long long &kickout_out, long long &kickout_dirty_out, long long &complex_updates_out) {
   kickout_out = kickout;
   kickout_dirty_out = kickout_dirty;
   complex_updates_out = complex_updates;
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::watch_print(ostream &output) {
   output << "There are " << rc_data.size() << " valid entries in the range cache. " << endl;
   for (unsigned int i = 0; i < rc_data.size() ; i++) {
      output << "start_addr = " << setw(10) << rc_data[i].start_addr << " ";
      output << "end_addr = " << setw(10) << rc_data[i].end_addr << " ";
      if (rc_data[i].flags & WA_READ)
         output << "R";
      if (rc_data[i].flags & WA_WRITE)
         output << "W";
      output << endl;
   }
   output << endl;
   return;
}

#endif
