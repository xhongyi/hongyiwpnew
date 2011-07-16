#ifndef L1_CACHE_H_
#define L1_CACHE_H_

#include <deque>
using namespace std;

template<class ADDRESS>
class L1Cache {
public:
   L1Cache();
   ~L1Cache();
   void set_size(unsigned int L1_SET_IDX_LEN_in, unsigned int L1_BLOCK_OFFSET_in, unsigned int L1_ASSOCIATIVITY_in);
   // cache controller functions
   
   // basic cache operation functions
   bool check_and_update(ADDRESS target_index);
   void update_if_exist(ADDRESS target_index);
   // statistics
private:
   unsigned int L1_SET_IDX_LEN, L1_BLOCK_OFFSET, L1_ASSOCIATIVITY;
   unsigned int L1_SET_NUM, L1_BLOCK_SIZE;
   deque<ADDRESS> cache_data[L1_SET_NUM];
   // basic cache operation functions
   bool cache_overflow(unsigned int set);
   void cache_kickout(unsigned int set);
};

#include "l1_cache.cpp"
#endif /* L1_CACHE_H_ */
