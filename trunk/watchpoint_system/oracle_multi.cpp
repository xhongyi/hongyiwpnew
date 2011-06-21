#ifndef ORACLE_MULTI_CPP_
#define ORACLE_MULTI_CPP_

#include "oracle_multi.h"

template<class ADDRESS, class FLAGS>
Oracle_multi<ADDRESS, FLAGS>::Oracle_multi() {
}

template<class ADDRESS, class FLAGS>
Oracle_multi<ADDRESS, FLAGS>::~Oracle_multi() {
}

template<class ADDRESS, class FLAGS>
void Oracle_multi<ADDRESS, FLAGS>::start_thread(int32_t thread_id, Oracle<ADDRESS, FLAGS>* oracle_in) {
   oracle_wp[thread_id] = oracle_in;
}

template<class ADDRESS, class FLAGS>
void Oracle_multi<ADDRESS, FLAGS>::end_thread(int32_t thread_id, Oracle<ADDRESS, FLAGS>* oracle_in) {
   oracle_wp.erase(thread_id);
   if (oracle_wp.size() == 0)
      wp.rm_watchpoint(0, (ADDRESS)-1, WA_READ | WA_WRITE);
   else if (oracle_wp.size() == 1)
      wp = *(oracle_wp.begin()->second);
   else {
      watchpoint_t<ADDRESS, FLAGS> temp = oracle_in->start_traverse();
      do {
         rm_watchpoint(temp.start_addr, temp.end_addr, temp.flags);
      } while (oracle_in->continue_traverse(temp));
   }
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   return wp.general_fault(start_addr, end_addr, target_flags);
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return wp.watch_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return wp.read_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return wp.write_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   return wp.add_watchpoint(start_addr, end_addr, target_flags);
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   bool fault = false;
   if (oracle_wp.size() == 1) {
      wp.rm_watchpoint(start_addr, end_addr, target_flags);
   }
   if ((target_flags & WA_READ)) {
      ADDRESS i=start_addr;
      do {
         fault = false;
         for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter!=oracle_wp.end();oracle_wp_iter++) {
            if (oracle_wp_iter->second->read_fault(i, i)) {
               fault = true;
               break;
            }
         }
         if (!fault)
            wp.rm_watchpoint(i, i, WA_READ);
         i++;
      } while (i!=end_addr+1);
   }
   if (target_flags & WA_WRITE) {
      ADDRESS i=start_addr;
      do {
         for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter!=oracle_wp.end();oracle_wp_iter++) {
            if (oracle_wp_iter->second->write_fault(i, i)) {
               fault = true;
               break;
            }
         }
         if (!fault)
            wp.rm_watchpoint(i, i, WA_WRITE);
         i++;
      } while (i!=end_addr+1);
   }
   return 0;
}



#endif
