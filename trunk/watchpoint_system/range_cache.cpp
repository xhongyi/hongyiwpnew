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
int RangeCache<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   return wp_operation(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   return wp_operation(start_addr, end_addr);
}
// we only need to know if it is a hit or miss in a range cache
//    so it is regardless of the checked flags
template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, bool dirty) {
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_read_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   int rc_miss = 0;
   bool searching = true;     // searching = true until all ranges are covered
   while (searching) {
      rc_read_iter = search_address(start_addr);               // search starts from the start_addr
      if (rc_read_iter == rc_data.end()) {                     // if cache miss
         rc_miss++;                                            //    miss++
         // get new range from backing store
         rc_read_iter = oracle_wp->search_address(start_addr);
         temp = *rc_read_iter;
         if (dirty)
            temp.flags |= DIRTY;
         rc_data.push_back(*rc_read_iter);                     // push the new range to range cache
         rc_read_iter = search_address(start_addr);
      }
      if (rc_read_iter->end_addr >= end_addr)                  // if all ranges are covered
         searching = false;
      // refresh start_addr
      temp = *rc_read_iter;
      start_addr = temp.end_addr+1;
      // mark as dirty if called by wp_operation
      if (dirty)
         temp.flags |= DIRTY;// mark as dirty if called by wp_operation
      // refresh lru
      rc_data.erase(rc_read_iter);
      rc_data.push_back(temp);
   }
   while (cache_overflow())
      cache_kickout();
   return rc_miss;
}
// wp_operation is same for add or rm a watchpoint, 
//    both simulated by removing all covered ranges and then getting new ranges from backing store
template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr) {
   ADDRESS new_start_addr = start_addr, new_end_addr = end_addr;
   bool complex_update = false;
   int rc_miss = 0;
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_write_iter;
   // extend start_addr if necessary
   rc_write_iter = search_address(start_addr);
   if (rc_write_iter != rc_data.end()) {        // in case of split
      new_start_addr = rc_write_iter->start_addr;
   }
   else 
      complex_update = true;
   rc_write_iter = oracle_wp->search_address(start_addr);
   if (rc_write_iter->start_addr < start_addr)  // in case of merge
      start_addr = rc_write_iter->start_addr;
   // check if complex_update
   if (rc_write_iter->end_addr != (ADDRESS)-1) {
      rc_write_iter++;
      if (rc_write_iter->end_addr<end_addr)
         complex_update = true;
   }
   // extend end_addr if necessary
   rc_write_iter = search_address(end_addr);
   if (rc_write_iter != rc_data.end()) {        // in case of split
      new_end_addr = rc_write_iter->end_addr;
   }
   else
      complex_update = true;
   rc_write_iter = oracle_wp->search_address(end_addr);
   if (rc_write_iter->end_addr > end_addr)      // in case of merge
      end_addr = rc_write_iter->end_addr;
   // getting statistics for backing store accesses
   rc_miss = general_fault(start_addr, end_addr);
   // rm all entries within the new range
   rm_range(new_start_addr, new_end_addr);
   // update these entries
   general_fault(new_start_addr, new_end_addr, true);
   if (complex_update)
      complex_updates++;
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
   if (rc_data.front().flags & DIRTY)
      kickout_dirty++;
   rc_data.pop_front();
}

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
   bool searching = true;  // searching = true until all ranges are removed
   while (searching) {
      rc_rm_iter = search_address(start_addr);
      if (rc_rm_iter != rc_data.end()) {        // if found in range cache
         if (rc_rm_iter->end_addr >= end_addr)  // if finished all ranges
            searching = false;
         start_addr = rc_rm_iter->end_addr+1;   // adjust start_addr
         rc_data.erase(rc_rm_iter);
      }                                         // if miss in range cache
      else if (start_addr != end_addr)
         start_addr++;
      else
         searching = false;
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
