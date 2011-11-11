#ifndef MEM_TRACKER_H_
#define MEM_TRACKER_H_
// cache size for mem_tracker (can be modified for wlb in off-chip bitmap)
// Each line in the TL1 is 64-bytes long. Because it takes 2 bits to hold
// a watchpoint for a byte in memory, each line holds 256 words of watchpoints.
#define LOG_CACHE_LINE_SIZE      8UL
#define CACHE_LINE_SIZE          (1<<LOG_CACHE_LINE_SIZE) // 256
// 4-way set associative
#define CACHE_ASSOCIATIVITY      4UL
// 4KB, 4-way SA cache with 64byte lines means 16 lines per way.
#define CACHE_SET_IDX_LEN        4UL
#define CACHE_SET_NUM            (1<<CACHE_SET_IDX_LEN)    // 16
// Size of the bus to memory, in bytes, to count how many loads it takes to fill a cache line.
#define MEM_BUS_SIZE             8UL
#define NUM_MEMORY_CYCLES        (CACHE_LINE_SIZE>>2)/MEM_BUS_SIZE

#include <iostream>
#include <deque>
using namespace std;

template<class ADDRESS>
class MemTracker {
public:
   MemTracker(ostream &output_stream, int create_thread_id);
   MemTracker();
   ~MemTracker();
   // returns the number of misses in WLB
   unsigned int general_fault     (ADDRESS start_addr, ADDRESS end_addr);
   unsigned int wp_operation      (ADDRESS start_addr, ADDRESS end_addr);
private:
   deque<ADDRESS>      cache[CACHE_SET_NUM];
   // returns true if it is a hit
   bool check_and_update (ADDRESS target_index);
   void update_if_exist  (ADDRESS target_index);
   void evict_if_exist (ADDRESS target_index);
   // for keeping the size of the WLB
   bool cache_overflow (ADDRESS set);
   void cache_kickout  (ADDRESS set);
   ostream &trace_output;
   int thread_id;
};

#include "mem_tracker.cpp"
#endif /*MEM_TRACKER_H_*/
