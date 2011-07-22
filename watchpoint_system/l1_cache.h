#ifndef L1_CACHE_H_
#define L1_CACHE_H_

#define L1_SET_IDX_LEN     4
#define L1_SET_NUM         (1<<L1_SET_IDX_LEN)
#define L1_BLOCK_OFFSET    4
#define L1_BLOCK_SIZE      (1<<L1_BLOCK_OFFSET)
#define L1_ASSOCIATIVITY   4

#include <deque>
using namespace std;

template<class ADDRESS>
class L1Cache {
public:
   L1Cache();
   ~L1Cache();
   // cache controller functions
   
   // basic cache operation functions
   bool check_and_update(ADDRESS target_index);
   void update_if_exist(ADDRESS target_index);
   // statistics
private:
   deque<ADDRESS>* cache_data;
   // basic cache operation functions
   bool cache_overflow(unsigned int set);
   void cache_kickout(unsigned int set);
};

#include "l1_cache.cpp"
#endif /* L1_CACHE_H_ */
