#ifndef L2_CACHE_H_
#define L2_CACHE_H_

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
   void set_size(unsigned int L2_SET_IDX_LEN_in, unsigned int L2_BLOCK_OFFSET_in, unsigned int L2_ASSOCIATIVITY_in, unsigned int VICTIM_CACHE_SIZE_in);
   // cache controller functions
   
   // basic cache operation functions
   bool check_and_update(int32_t thread_id, ADDRESS target_index);
   void update_if_exist(int32_t thread_id, ADDRESS target_index);
   // statistics
private:
   unsigned int L2_SET_IDX_LEN, L2_BLOCK_OFFSET, L2_ASSOCIATIVITY;
   unsigned int L2_SET_NUM, L2_BLOCK_SIZE, VICTIM_CACHE_SIZE;
   deque<L2CacheEntry<ADDRESS> > cache_data[L2_SET_NUM];
   deque<L2CacheEntry<ADDRESS> > victim_cache;
   // basic cache operation functions
   bool cache_overflow(unsigned int set);
   void cache_kickout(unsigned int set);
};

#include "l2_cache.cpp"
#endif /* L2_CACHE_H_ */
