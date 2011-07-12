#ifndef RANGE_CACHE_CPP_
#define RANGE_CACHE_CPP_

#include "range_cache.h"

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::RangeCache(Oracle<ADDRESS, FLAGS> *wp_ref, bool ocbm_in, bool offcbm_in) {
   ocbm = ocbm_in;
   off_cbm = offcbm_in;
   oracle_wp = wp_ref;
   if (off_cbm)
      offcbm_wp = new Offcbm<ADDRESS, FLAGS>(oracle_wp);
   kickout_dirty=0;
   kickout=0;
   complex_updates=0;
   offcbm_switch=0;
   range_switch=0;
}

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::RangeCache() {
   ocbm = false;
   off_cbm = false;
   kickout_dirty=0;
   kickout=0;
   complex_updates=0;
   offcbm_switch=0;
   range_switch=0;
}

template<class ADDRESS, class FLAGS>
RangeCache<ADDRESS, FLAGS>::~RangeCache() {
   if (off_cbm)
      delete offcbm_wp;
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
   ADDRESS search_addr = start_addr;
   while (searching) {
      rc_read_iter = search_address(search_addr);              // search starts from the start_addr
      if (rc_read_iter == rc_data.end()) {                     // if cache miss
         // get new range from backing store
         /* added for off-cbm */
         if (off_cbm)
            rc_read_iter = offcbm_wp->search_address(search_addr);
         else
         /* added for off-cbm */
            rc_read_iter = oracle_wp->search_address(search_addr);
         rc_miss += rm_range(rc_read_iter->start_addr, rc_read_iter->end_addr);
         /* added for off-cbm */
         if (off_cbm && (rc_read_iter->flags & WA_OFFCBM)) {
            rc_miss += offcbm_wp->general_fault(  max(search_addr, rc_read_iter->start_addr)
                                                , min(end_addr,    rc_read_iter->end_addr  ) );  // refresh wlb
         }
         /* added for off-cbm */
         rc_data.push_front(*rc_read_iter);                    // push the new range to range cache
         rc_read_iter = search_address(search_addr);
      }
      if (rc_read_iter->end_addr >= end_addr)                  // if all ranges are covered
         searching = false;
      // refresh start_addr
      temp = *rc_read_iter;
      search_addr = temp.end_addr+1;
      // refresh lru
      rc_data.erase(rc_read_iter);                             // refresh this entry as most recently used
      rc_data.push_front(temp);
   }
   if (ocbm)
      check_ocbm(start_addr, end_addr);
   while (cache_overflow())                                    // kick out redundant cache entries
      cache_kickout();
   return rc_miss;
}
// wp_operation is same for add or rm a watchpoint, 
//    both simulated by removing all covered ranges and then getting new ranges from backing store
template<class ADDRESS, class FLAGS>
int RangeCache<ADDRESS, FLAGS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr, bool is_update) {
   int rc_miss = 0;
   bool complex_update = false;
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_write_iter, oracle_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   if (is_update) {
      // support for counting complex updates
      rc_write_iter = search_address(start_addr);
      if (rc_write_iter == rc_data.end())
         complex_update = true;
      rc_write_iter = search_address(end_addr);
      if (rc_write_iter == rc_data.end())
         complex_update = true;
      if ( (oracle_wp->search_address(start_addr)+1)->end_addr < end_addr)
         complex_update = true;
      if (complex_update)
         complex_updates++;
      // counting rc_miss
      bool searching = true;     // searching = true until all ranges are covered
      ADDRESS search_addr = start_addr;
      while (searching) {
         rc_write_iter = search_address(search_addr);             // search starts from the start_addr
         /* added for off-cbm */
         if (off_cbm)
            oracle_iter = offcbm_wp->search_address(search_addr);
         else
         /* added for off-cbm */
            oracle_iter = oracle_wp->search_address(search_addr);
         /* added for off-cbm */
         if (off_cbm && (oracle_iter->flags & WA_OFFCBM)) {
            rc_miss += offcbm_wp->general_fault(  max(search_addr, oracle_iter->start_addr)
                                                , min(end_addr,    oracle_iter->end_addr  ) );  // refresh wlb
         }
         /* added for off-cbm */
         if (rc_write_iter != rc_data.end()) {                    // if cache hit
            if (rc_write_iter->end_addr >= end_addr)
               searching = false;
            else
               search_addr = rc_write_iter->end_addr+1;
         }
         else {
            if (oracle_iter->end_addr >= end_addr) {
               rc_miss += rm_range(search_addr, end_addr);
               searching = false;
            }
            else {
               rc_miss += rm_range(search_addr, oracle_iter->end_addr);
               search_addr = oracle_iter->end_addr+1;
            }
         }
      }
      rm_range(start_addr, end_addr);
      // adding ranges
      oracle_iter = oracle_wp->search_address(start_addr);     // merge start
      temp = *oracle_iter;
      temp.flags |= DIRTY;
      if (oracle_iter->start_addr < start_addr) {
         rc_write_iter = search_address(start_addr-1);
         if (rc_write_iter!=rc_data.end() && ((rc_write_iter->flags & WA_OFFCBM) == 0) ) {
            temp.start_addr = rc_write_iter->start_addr;
            rc_data.erase(rc_write_iter);
         }
         else
            temp.start_addr = start_addr;
      }
      if (oracle_iter->end_addr > end_addr) {
         rc_write_iter = search_address(end_addr+1);
         if (rc_write_iter!=rc_data.end() && ((rc_write_iter->flags & WA_OFFCBM) == 0) ) {
            temp.end_addr = rc_write_iter->end_addr;
            rc_data.erase(rc_write_iter);
         }
         else
            temp.end_addr = end_addr;
      }
      rc_data.push_front(temp);
      if (oracle_iter->end_addr < end_addr) {
         searching = true;
         while (searching) {
            oracle_iter++;
            temp = *oracle_iter;
            temp.flags |= DIRTY;
            if (oracle_iter->end_addr > end_addr) {            // merge end
               searching = false;
               rc_write_iter = search_address(end_addr+1);
               if (rc_write_iter!=rc_data.end() && ((rc_write_iter->flags & WA_OFFCBM) == 0) ) {
                  temp.end_addr = rc_write_iter->end_addr;
                  rc_data.erase(rc_write_iter);
               }
               else
                  temp.end_addr = end_addr;
            }
            else if (oracle_iter->end_addr == end_addr)
               searching = false;
            rc_data.push_front(temp);
         }
      }
   }
   else {      // sets can create only one range
      rm_range(start_addr, end_addr);
      if (off_cbm)
         offcbm_wp->rm_offcbm(start_addr, end_addr);  // after a set watchpoint, everything within 
                                                      // the range should better become a range
      oracle_iter = oracle_wp->search_address(start_addr);
      temp = *oracle_iter;
      temp.flags |= DIRTY;
      if (oracle_iter->start_addr < start_addr) {     // merge start
         rc_write_iter = search_address(start_addr-1);
         if (rc_write_iter!=rc_data.end() && ((rc_write_iter->flags & WA_OFFCBM) == 0) ) {
            temp.start_addr = rc_write_iter->start_addr;
            rc_data.erase(rc_write_iter);
         }
         else
            temp.start_addr = start_addr;
      }
      if (oracle_iter->end_addr > end_addr) {         // merge end
         rc_write_iter = search_address(end_addr+1);
         if (rc_write_iter!=rc_data.end() && ((rc_write_iter->flags & WA_OFFCBM) == 0) ) {
            temp.end_addr = rc_write_iter->end_addr;
            rc_data.erase(rc_write_iter);
         }
         else
            temp.end_addr = end_addr;
      }
      rc_data.push_front(temp);
   }
   if (ocbm)
      check_ocbm(start_addr, end_addr);
   while (cache_overflow())                           // kick out redundant cache entries
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
   if (off_cbm) {
      kickout++;
      if (rc_data.back().flags & DIRTY) {
         kickout_dirty++;
         int check_switch = offcbm_wp->kickout_dirty(rc_data.back().start_addr);
         if (check_switch == 1) {      // switch to offcbm
            offcbm_switch++;
            general_fault(rc_data.back().start_addr, rc_data.back().start_addr);
         }
         else if (check_switch == 2) { // switch to ranges
            range_switch++;
            rc_data.pop_back();
         }
         else
            rc_data.pop_back();
      }
      else
         rc_data.pop_back();
   }
   else {
      kickout++;
      if (rc_data.back().flags & DIRTY)
         kickout_dirty++;
      rc_data.pop_back();
   }
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
int RangeCache<ADDRESS, FLAGS>::rm_range(ADDRESS start_addr, ADDRESS end_addr) {
   int rc_miss = 0;
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator rc_rm_iter;
   watchpoint_t<ADDRESS, FLAGS> temp;
   rc_rm_iter = search_address(start_addr);
   // split start if needed
   if (rc_rm_iter!=rc_data.end() && rc_rm_iter->start_addr<start_addr) {
      temp.start_addr = rc_rm_iter->start_addr;
      temp.end_addr = start_addr-1;
      temp.flags = rc_rm_iter->flags;
      if (ocbm && (temp.end_addr - temp.start_addr <= OCBM_LEN-1))
         temp.flags |= WA_OCBM;
      rc_rm_iter->start_addr = start_addr;
      rc_data.push_front(temp);
   }
   rc_rm_iter = search_address(end_addr);
   if (rc_rm_iter == rc_data.end())
      rc_miss++;
   // split end if needed
   if (rc_rm_iter!=rc_data.end() && rc_rm_iter->end_addr>end_addr) {
      temp.start_addr = end_addr+1;
      temp.end_addr = rc_rm_iter->end_addr;
      temp.flags = rc_rm_iter->flags;
      if (ocbm && (temp.end_addr - temp.start_addr <= OCBM_LEN-1))
         temp.flags |= WA_OCBM;
      rc_rm_iter->end_addr = end_addr;
      rc_data.push_front(temp);
   }
   unsigned int i=0;
   while (i < rc_data.size()) {
      rc_rm_iter = rc_data.begin()+i;
      if (rc_rm_iter->start_addr >= start_addr && rc_rm_iter->start_addr <= end_addr && 
          rc_rm_iter->end_addr   >= start_addr && rc_rm_iter->end_addr   <= end_addr) {
         rc_data.erase(rc_rm_iter);
         rc_miss++;
      }
      else
         i++;
   }
   return rc_miss;
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::watch_print(ostream &output) {
   output << "There are " << rc_data.size() << " valid entries in the range cache. " << endl;
   for (unsigned int i = 0; i < rc_data.size() ; i++) {
      output << "start_addr = " << setw(10) << rc_data[i].start_addr << " ";
      output << "end_addr = " << setw(10) << rc_data[i].end_addr << " ";
      if (rc_data[i].flags & WA_OFFCBM) {
         output << "OFFCBM";
      }
      else if (rc_data[i].flags & WA_OCBM) {
         output << "OCBM";
      }
      else {
         if (rc_data[i].flags & WA_READ)
            output << "R";
         if (rc_data[i].flags & WA_WRITE)
            output << "W";
      }
      output << endl;
   }
   output << endl;
   return;
}

template<class ADDRESS, class FLAGS>
void RangeCache<ADDRESS, FLAGS>::check_ocbm(ADDRESS start_addr, ADDRESS end_addr) {
   typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator check_iter, merge_iter;
   // start from the range before start_addr in case of a split
   check_iter = search_address(start_addr-1);
   if (check_iter != rc_data.end())
      start_addr = check_iter->start_addr;
   ADDRESS search_addr = start_addr;
   // switch all qualified ranges into ocbm first
   bool searching = true;
   do {
      check_iter = search_address(search_addr);                // is guaranteed to be in the cache
      if (check_iter->end_addr - check_iter->start_addr <= OCBM_LEN-1)
         check_iter->flags |= WA_OCBM;
      if (check_iter->end_addr >= end_addr)
         searching = false;
      else
         search_addr = check_iter->end_addr+1;
   } while (searching);
   // check their neighbors for merge
   searching = true;
   search_addr = start_addr;
   do {
      check_iter = search_address(search_addr);
      if (check_iter->flags & WA_OCBM) {                       // only merge if it is an ocbm itself
         // merge left
         merge_iter = search_address(check_iter->start_addr-1);
         if (merge_iter!=rc_data.end() && (merge_iter->flags & WA_OCBM)) {
            if (check_iter->end_addr - merge_iter->start_addr <= OCBM_LEN-1) {
               check_iter->flags |= merge_iter->flags;         // in case of merging dirty ocbms
               check_iter->start_addr = merge_iter->start_addr;
               rc_data.erase(merge_iter);
               check_iter = search_address(search_addr);
            }
         }
         if (check_iter->end_addr >= end_addr)
            searching = false;
         else {
            search_addr = check_iter->end_addr+1;
            // merge right
            merge_iter = search_address(search_addr);
            if (merge_iter!=rc_data.end() && (merge_iter->flags & WA_OCBM)) {
               if (merge_iter->end_addr - check_iter->start_addr <= OCBM_LEN-1) {
                  check_iter->flags |= merge_iter->flags;
                  check_iter->end_addr = merge_iter->end_addr;
                  if (merge_iter->end_addr >= end_addr)
                     searching = false;
                  else
                     search_addr = merge_iter->end_addr+1;
                  rc_data.erase(merge_iter);
               }
            }
         }
      }
      else {
         if (check_iter->end_addr >= end_addr)
            searching = false;
         else
            search_addr = check_iter->end_addr+1;
      }
   } while (searching);
}

#endif
