#ifndef L2_CACHE_CPP_
#define L2_CACHE_CPP_

#include "l2_cache.h"

template<class ADDRESS>
L2Cache<ADDRESS>::L2Cache() {
}

template<class ADDRESS>
L2Cache<ADDRESS>::~L2Cache() {
}

template<class ADDRESS>
void L2Cache<ADDRESS>::set_size(unsigned int L2_SET_IDX_LEN_in, unsigned int L2_BLOCK_OFFSET_in, unsigned int L2_ASSOCIATIVITY_in, unsigned int VICTIM_CACHE_SIZE_in) {
   L2_SET_IDX_LEN = L2_SET_IDX_LEN_in;
   L2_BLOCK_OFFSET = L2_BLOCK_OFFSET_in;
   L2_ASSOCIATIVITY = L2_ASSOCIATIVITY_in;
   L2_SET_NUM = (1<<L2_SET_IDX_LEN);
   L2_BLOCK_SIZE = (1<<L2_BLOCK_OFFSET);
   VICTIM_CACHE_SIZE = VICTIM_CACHE_SIZE_in;
}

template<class ADDRESS>
bool L2Cache<ADDRESS>::check_and_update(int32_t thread_id, ADDRESS target_index) {
   typename deque<L2CacheEntry<ADDRESS> >::iterator i;
   L2CacheEntry<ADDRESS> temp;
   temp.tag = tag;
   temp.thread_id = thread_id;
   ADDRESS set = target_index & (L2_SET_NUM-1);
   ADDRESS tag = (target_index >> L2_SET_IDX_LEN);
   deque<L2CacheEntry<ADDRESS> >* cur_set = &cache_data[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == temp) {
         cur_set->erase(i);
         cur_set->push_front(temp);
         return true;
      }
   }
   cur_set->push_front(temp);
   if (cache_overflow(set))
      cache_kickout(set);
   return false;
}

template<class ADDRESS>
void L2Cache<ADDRESS>::update_if_exist(int32_t thread_id, ADDRESS target_index) {
   typename deque<L2CacheEntry<ADDRESS> >::iterator i;
   L2CacheEntry<ADDRESS> temp;
   temp.tag = tag;
   temp.thread_id = thread_id;
   ADDRESS set = target_index & (L2_SET_NUM-1);
   ADDRESS tag = (target_index >> L2_SET_IDX_LEN);
   deque<L2CacheEntry<ADDRESS> >* cur_set = &cache_data[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == temp) {
         cur_set->erase(i);
         cur_set->push_front(temp);
         return;
      }
   }
}

template<class ADDRESS>
inline bool L2Cache<ADDRESS>::cache_overflow(unsigned int set) {
   return (cache_data[set].size() > L2_ASSOCIATIVITY);
}

template<class ADDRESS>
inline void L2Cache<ADDRESS>::cache_kickout(unsigned int set) {
   L2CacheEntry<ADDRESS> temp = cache_data[set].back();
   cache_data[set].pop_back();
   victim_cache.push_front(temp);
   // in Virtual Memory: mark the page as unavailable if necessary (i.e. check it before set it)
}

#endif /* L2_CACHE_CPP_ */
