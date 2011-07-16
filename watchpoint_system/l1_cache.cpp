#ifndef L1_CACHE_CPP_
#define L1_CACHE_CPP_

#include "l1_cache.h"

template<class ADDRESS>
L1Cache<ADDRESS>::L1Cache() {
}

template<class ADDRESS>
L1Cache<ADDRESS>::~L1Cache() {
}

template<class ADDRESS>
void L1Cache<ADDRESS>::set_size(unsigned int L1_SET_IDX_LEN_in, unsigned int L1_BLOCK_OFFSET_in, unsigned int L1_ASSOCIATIVITY_in) {
   L1_SET_IDX_LEN = L1_SET_IDX_LEN_in;
   L1_BLOCK_OFFSET = L1_BLOCK_OFFSET_in;
   L1_ASSOCIATIVITY = L1_ASSOCIATIVITY_in;
   L1_SET_NUM = (1<<L1_SET_IDX_LEN);
   L1_BLOCK_SIZE = (1<<L1_BLOCK_OFFSET);
}

template<class ADDRESS>
bool L1Cache<ADDRESS>::check_and_update(ADDRESS target_index) {
   typename deque<ADDRESS>::iterator i;
   ADDRESS set = target_index & (L1_SET_NUM-1);
   ADDRESS tag = (target_index >> L1_SET_IDX_LEN);
   deque<ADDRESS>* cur_set = &cache_data[set];
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
void L1Cache<ADDRESS>::update_if_exist(ADDRESS target_index) {
   typename deque<ADDRESS>::iterator i;
   ADDRESS set = target_index & (L1_SET_NUM-1);
   ADDRESS tag = (target_index >> L1_SET_IDX_LEN);
   deque<ADDRESS>* cur_set = &cache_data[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == tag) {
         cur_set->erase(i);
         cur_set->push_front(tag);
         return;
      }
   }
}

template<class ADDRESS>
inline bool L1Cache<ADDRESS>::cache_overflow(unsigned int set) {
   return (cache_data[set].size() > L1_ASSOCIATIVITY);
}

template<class ADDRESS>
inline void L1Cache<ADDRESS>::cache_kickout(unsigned int set) {
   cache_data[set].pop_back();
}

#endif /* L1_CACHE_CPP_ */
