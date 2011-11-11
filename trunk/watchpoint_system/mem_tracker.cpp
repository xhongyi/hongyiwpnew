#ifndef MEM_TRACKER_CPP_
#define MEM_TRACKER_CPP_

#include "mem_tracker.h"
template<class ADDRESS>
MemTracker<ADDRESS>::MemTracker(ostream &output_stream, int create_thread_id)
    : trace_output(output_stream)
{
    thread_id = create_thread_id;
}

template<class ADDRESS>
MemTracker<ADDRESS>::MemTracker()
    : trace_output(cout)
{
}

template<class ADDRESS>
MemTracker<ADDRESS>::~MemTracker() {
}

template<class ADDRESS>
unsigned int MemTracker<ADDRESS>::general_fault(ADDRESS start_addr, ADDRESS end_addr) {
   unsigned int miss = 0;
   ADDRESS start_idx = (start_addr>>LOG_CACHE_LINE_SIZE);
   ADDRESS end_idx   = (end_addr  >>LOG_CACHE_LINE_SIZE)+1;
   for (ADDRESS i=start_idx;i!=end_idx;i++) {
      if (!check_and_update(i)) {                 // if miss in the cache, get immediately from main memory
         miss++;
#ifdef PRINT_RANGE_TRACE
         if(total_print_number < MAX_PRINT_NUM) {
             total_print_number++;
             trace_output << 'o' << setw(5) << thread_id << setw(8) << start_addr << setw(8) << end_addr << endl;
         }
#endif
      }
   }
   // Each cache line fill takes 8 loads from main memory to complete.
   if (NUM_MEMORY_CYCLES*miss >= miss) // we use the = because miss might be 0
      return NUM_MEMORY_CYCLES*miss;
   else {
       return 0xffffffff;
   }
}

template<class ADDRESS>
unsigned int MemTracker<ADDRESS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr) {
   ADDRESS start_idx = (start_addr>>LOG_CACHE_LINE_SIZE);
   ADDRESS end_idx   = (end_addr  >>LOG_CACHE_LINE_SIZE)+1;
   unsigned int miss = end_idx - start_idx;     // write through to main memory
#ifdef RC_OFFCBM
#ifdef PRINT_RANGE_TRACE
   if(total_print_number < MAX_PRINT_NUM) {
       total_print_number++;
       trace_output << 'n' << setw(5) << thread_id << setw(8) << start_addr << setw(8) << end_addr << endl;
   }
#endif
   for (ADDRESS i=start_idx; i != end_idx; i++)
      evict_if_exist(i);
#else // !RC_OFFCBM
   for (ADDRESS i=start_idx;i!=end_idx;i++)
      update_if_exist(i);                       // update lru only if it exist (not write allocate)
#endif
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
void MemTracker<ADDRESS>::update_if_exist(ADDRESS target_index) {
   typename deque<ADDRESS>::iterator i;
   ADDRESS set = target_index & (CACHE_SET_NUM-1);
   ADDRESS tag = (target_index >> CACHE_SET_IDX_LEN);
   deque<ADDRESS>* cur_set = &cache[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == tag) {
         cur_set->erase(i);
         cur_set->push_front(tag);
         return;
      }
   }
}

template<class ADDRESS>
void MemTracker<ADDRESS>::evict_if_exist(ADDRESS target_index) {
   typename deque<ADDRESS>::iterator i;
   ADDRESS set = target_index & (CACHE_SET_NUM-1);
   ADDRESS tag = (target_index >> CACHE_SET_IDX_LEN);
   deque<ADDRESS>* cur_set = &cache[set];
   for (i=cur_set->begin();i!=cur_set->end();i++) {
      if (*i == tag) {
         cur_set->erase(i);
         return;
      }
   }
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
