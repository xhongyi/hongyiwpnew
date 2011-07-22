#ifndef L2_CACHE_CPP_
#define L2_CACHE_CPP_

#include "l2_cache.h"

template<class ADDRESS>
L2Cache<ADDRESS>::L2Cache() {
   cache_data = new deque<L2CacheEntry<ADDRESS> > [L2_SET_NUM];
}

template<class ADDRESS>
L2Cache<ADDRESS>::~L2Cache() {
   for (int i=0;i<L2_SET_NUM;i++) {
      cache_data[i].clear();
   }
   delete [] cache_data;
   victim_cache.clear();
}

template<class ADDRESS>
bool L2Cache<ADDRESS>::check_and_update(int32_t thread_id, ADDRESS target_index) {
   typename deque<L2CacheEntry<ADDRESS> >::iterator i;
   L2CacheEntry<ADDRESS> temp;
   ADDRESS set = target_index & (L2_SET_NUM-1);
   ADDRESS tag = (target_index >> L2_SET_IDX_LEN);
   temp.tag = tag;
   temp.thread_id = thread_id;
   deque<L2CacheEntry<ADDRESS> >* cur_set = &(cache_data[set]);
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if ((i->tag == temp.tag) && (i->thread_id == temp.thread_id)) {
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
   ADDRESS set = target_index & (L2_SET_NUM-1);
   ADDRESS tag = (target_index >> L2_SET_IDX_LEN);
   temp.tag = tag;
   temp.thread_id = thread_id;
   deque<L2CacheEntry<ADDRESS> >* cur_set = &(cache_data[set]);
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if ((i->tag == temp.tag) && (i->thread_id == temp.thread_id)) {
         cur_set->erase(i);
         cur_set->push_front(temp);
         return;
      }
   }
   temp.tag = target_index;
   for (i=victim_cache.begin();i!=victim_cache.end();i++) {
      if ((i->tag == temp.tag) && (i->thread_id == temp.thread_id)) {
         victim_cache.erase(i);
         victim_cache.push_front(temp);
         return;
      }
   }
}

template<class ADDRESS>
bool L2Cache<ADDRESS>::victim_check_and_rm(int32_t thread_id, ADDRESS target_index) {
   typename deque<L2CacheEntry<ADDRESS> >::iterator i;
   for (i=victim_cache.begin();i!=victim_cache.end();i++) {
      if ((i->thread_id == thread_id) && (i->tag == target_index)) {
         victim_cache.erase(i);
         return true;
      }
   }
   return false;
}

template<class ADDRESS>
inline bool L2Cache<ADDRESS>::victim_overflow() {
   return (victim_cache.size() > VICTIM_CACHE_SIZE);
}

template<class ADDRESS>
inline void L2Cache<ADDRESS>::victim_kickout() {
   writeback_buffer=victim_cache.back();
   victim_cache.pop_back();
}

template<class ADDRESS>
inline bool L2Cache<ADDRESS>::cache_overflow(unsigned int set) {
   return (cache_data[set].size() > L2_ASSOCIATIVITY);
}

template<class ADDRESS>
inline void L2Cache<ADDRESS>::cache_kickout(unsigned int set) {
   L2CacheEntry<ADDRESS> temp = cache_data[set].back();
   cache_data[set].pop_back();
   temp.tag = ((temp.tag<<L2_SET_IDX_LEN) & set);
   victim_cache.push_front(temp);
}

#endif /* L2_CACHE_CPP_ */
