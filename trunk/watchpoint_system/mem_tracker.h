#ifndef MEM_TRACKER_H_
#define MEM_TRACKER_H_
// cache size for mem_tracker (can be modified for wlb in off-chip bitmap)
#define LOG_CACHE_ENTRY_SIZE     8UL
#define CACHE_ENTRY_SIZE         (1<<LOG_CACHE_ENTRY_SIZE) // 256
#define CACHE_ASSOCIATIVITY      4UL
#define CACHE_SET_IDX_LEN        4UL
#define CACHE_SET_NUM            (1<<CACHE_SET_IDX_LEN)    // 16

#include <deque>
using namespace std;

template<class ADDRESS>
class MemTracker {
public:
   MemTracker();
   ~MemTracker();
   // returns the number of misses in WLB
   int general_fault (ADDRESS start_addr, ADDRESS end_addr);
   int wp_operation  (ADDRESS start_addr, ADDRESS end_addr);
private:
   deque<ADDRESS>      cache[CACHE_SET_NUM];
   // returns true if it is a hit
   bool check_and_update (ADDRESS target_index);
   // for keeping the size of the WLB
   bool cache_overflow (ADDRESS set);
   void cache_kickout  (ADDRESS set);
};

#include "mem_tracker.cpp"
#endif /*MEM_TRACKER_H_*/
