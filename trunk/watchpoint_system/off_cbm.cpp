#ifndef OFF_CBM_CPP_
#define OFF_CBM_CPP_

#include "off_cbm.h"

template<class ADDRESS, class FLAGS>
Offcbm<ADDRESS, FLAGS>::Offcbm(Oracle<ADDRESS, FLAGS> *wp_ref) {
   oracle_wp = wp_ref;
}

template<class ADDRESS, class FLAGS>
Offcbm<ADDRESS, FLAGS>::Offcbm() {
}

template<class ADDRESS, class FLAGS>
Offcbm<ADDRESS, FLAGS>::~Offcbm() {
}

template<class ADDRESS, class FLAGS>
int Offcbm<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   return wlb.wp_operation(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int Offcbm<ADDRESS, FLAGS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr) {
   return wlb.wp_operation(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
Offcbm<ADDRESS, FLAGS>::search_address(ADDRESS target_addr) {
   watchpoint_t<ADDRESS, FLAGS> temp;
   ADDRESS tag = (target_addr>>LOG_OFF_CBM_SIZE);
   typename deque<ADDRESS>::iterator i;
   for (i=offcbm_pages.begin();i!=offcbm_pages.end();i++) {
      if (*i == tag) {
         ret_deq.clear();
         temp.start_addr = (tag<<LOG_OFF_CBM_SIZE);
         temp.end_addr   = (tag<<LOG_OFF_CBM_SIZE);
         temp.flags      = WA_OFFCBM;
         ret_deq.push_front(temp);
         return ret_deq.begin();
      }
   }
   return oracle_wp->search_address(target_addr);
}

template<class ADDRESS, class FLAGS>
int Offcbm<ADDRESS, FLAGS>::kickout_dirty(ADDRESS target_addr) {
   ADDRESS tag = (target_addr>>LOG_OFF_CBM_SIZE);
   typename deque<ADDRESS>::iterator i;
   for (i=offcbm_pages.begin();i!=offcbm_pages.end();i++) {
      if (*i == tag) {
         if (oracle_wp->search_address(((tag+1)<<LOG_OFF_CBM_SIZE)-1)
           - oracle_wp->search_address(tag<<LOG_OFF_CBM_SIZE)+1 < OFF_CBM_LOWER_THREASHOLD) {
            offcbm_pages.erase(i);
            return 2;
         }
         return 0;
      }
   }
   if (oracle_wp->search_address(((tag+1)<<LOG_OFF_CBM_SIZE)-1)
     - oracle_wp->search_address(tag<<LOG_OFF_CBM_SIZE)+1 > OFF_CBM_UPPER_THREASHOLD) {
      offcbm_pages.push_front(tag);
      return 1;
   }
   return 0;
}

template<class ADDRESS, class FLAGS>
void Offcbm<ADDRESS, FLAGS>::rm_offcbm(ADDRESS start_addr, ADDRESS end_addr) {
   ADDRESS start_idx = (start_addr>>LOG_OFF_CBM_SIZE);
   ADDRESS end_idx = (start_addr>>LOG_OFF_CBM_SIZE);
   typename deque<ADDRESS>::iterator i = offcbm_pages.begin();
   while (i == offcbm_pages.end()) {
      for (i=offcbm_pages.begin();i!=offcbm_pages.end();i++) {
         if (*i < end_idx && *i > start_idx) {
            offcbm_pages.erase(i);
            i=offcbm_pages.begin();
            break;
         }
      }
   }
}

#endif /*OFF_CBM_CPP_*/
