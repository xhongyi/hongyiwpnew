#ifndef MEM_TRACKER_CPP_
#define MEM_TRACKER_CPP_

#include "mem_tracker.h"

template<class ADDRESS>
MemTracker<ADDRESS>::MemTracker() {
}

template<class ADDRESS>
MemTracker<ADDRESS>::~MemTracker() {
}

template<class ADDRESS>
unsigned int MemTracker<ADDRESS>::general_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return wp_operation(start_addr, end_addr);
}

template<class ADDRESS>
unsigned int MemTracker<ADDRESS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr) {
   unsigned int miss = 0;
   ADDRESS start_idx = (start_addr>>LOG_CACHE_LINE_SIZE);
   ADDRESS end_idx   = (end_addr  >>LOG_CACHE_LINE_SIZE)+1;
   for (ADDRESS i=start_idx;i!=end_idx;i++) {
      if (!check_and_update(i))
         miss++;
   }
   // Each cache line fill takes 8 loads from main memory to complete.
   if (NUM_MEMORY_CYCLES*miss >= miss) // we use the = because miss might be 0
      return NUM_MEMORY_CYCLES*miss;
   else {
       return 0xffffffff;
   }
}

template<class ADDRESS>
bool MemTracker<ADDRESS>::check_and_update(ADDRESS target_index) {
   typename deque<ADDRESS>::iterator i;
   ADDRESS set = target_index & (CACHE_SET_NUM-1);
   ADDRESS tag = (target_index >> CACHE_SET_IDX_LEN);
   deque<ADDRESS>* cur_set = &cache[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == tag) {
         cur_set->erase(i);
         cur_set->push_front(tag);
         return true;
      }
   }
   cur_set->push_front(tag);
   if (cache_overflow(set))
      cache_kickout(set);
   return false;
}

template<class ADDRESS>
inline bool MemTracker<ADDRESS>::cache_overflow(ADDRESS set) {
   return (cache[set].size() > CACHE_ASSOCIATIVITY);
}

template<class ADDRESS>
inline void MemTracker<ADDRESS>::cache_kickout(ADDRESS set) {
   cache[set].pop_back();
}

#endif /*MEM_TRACKER_CPP_*/