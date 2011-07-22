#ifndef L2_CACHE_H_
#define L2_CACHE_H_

#define L2_SET_IDX_LEN     4
#define L2_SET_NUM         (1<<L2_SET_IDX_LEN)
#define L2_BLOCK_OFFSET    4
#define L2_BLOCK_SIZE      (1<<L2_BLOCK_OFFSET)
#define L2_ASSOCIATIVITY   4

#define VICTIM_CACHE_SIZE  4

#include <deque>
#include <stdint.h>
using namespace std;

template<class ADDRESS>
struct L2CacheEntry {
   ADDRESS tag;
   int32_t thread_id;
};

template<class ADDRESS>
class L2Cache {
public:
   L2Cache();
   ~L2Cache();
   // cache controller functions
   
   // basic cache operation functions
   bool check_and_update(int32_t thread_id, ADDRESS target_index);
   void update_if_exist(int32_t thread_id, ADDRESS target_index);
   bool victim_check_and_rm(int32_t thread_id, ADDRESS target_index);
   bool victim_overflow();
   void victim_kickout();
   // statistics
   L2CacheEntry<ADDRESS> writeback_buffer;
private:
   deque<L2CacheEntry<ADDRESS> >* cache_data;
   deque<L2CacheEntry<ADDRESS> > victim_cache;
   // basic cache operation functions
   bool cache_overflow(unsigned int set);
   void cache_kickout(unsigned int set);
};

#include "l2_cache.cpp"
#endif /* L2_CACHE_H_ */
