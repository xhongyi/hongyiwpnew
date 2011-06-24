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
void Oracle_multi<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   Oracle<ADDRESS, FLAGS>* temp_oracle = oracle_wp[thread_id];
   oracle_wp.erase(thread_id);
   /*if (oracle_wp.size() == 0)          // optimized for 1 threaded program
      wp.rm_watchpoint(0, (ADDRESS)-1, WA_READ | WA_WRITE);
   else if (oracle_wp.size() == 1)     // optimized for 2 threaded program
      wp = *(oracle_wp.begin()->second);
   else {
      watchpoint_t<ADDRESS, FLAGS> temp = oracle_in->start_traverse();
      do {
         rm_watchpoint(temp.start_addr, temp.end_addr, temp.flags);
      } while (oracle_in->continue_traverse(temp));
   }*/
   watchpoint_t<ADDRESS, FLAGS> temp = temp_oracle->start_traverse();
   do {
      if (temp.flags)
         rm_watchpoint(temp.start_addr, temp.end_addr, temp.flags);
   } while (temp_oracle->continue_traverse(temp));
   return;
}

template<class ADDRESS, class FLAGS>
int Oracle_multi<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator temp_deque_iter;
   wp.rm_watchpoint(start_addr, end_addr, target_flags);
   for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter!=oracle_wp.end();oracle_wp_iter++) {
      temp_deque_iter = oracle_wp_iter->second->search_address(start_addr);
      while (temp_deque_iter->start_addr <= end_addr) {
         if (temp_deque_iter->flags & target_flags)
            wp.add_watchpoint(start_addr, end_addr, temp_deque_iter->flags & target_flags);
         if (temp_deque_iter->end_addr != (ADDRESS)-1)
            temp_deque_iter++;
         else
            break;
      }
   }
   return 0;
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

#endif
