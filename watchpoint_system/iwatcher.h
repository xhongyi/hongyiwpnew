#ifndef IWATCHER_H_
#define IWATCHER_H_

#define L1_SET_IDX_LEN     4
#define L1_BLOCK_OFFSET    4
#define L1_ASSOCIATIVITY   4

#define L2_SET_IDX_LEN     4
#define L2_BLOCK_OFFSET    4
#define L2_ASSOCIATIVITY   4

#define VICTIM_CACHE_SIZE  4

#include "l1_cache.h"
#include "l2_cache.h"
#include "page_table1_single.h"

template<class ADDRESS, class FLAGS>
class IWatcher : public Virtual_wp<ADDRESS, FLAGS> {
public:
   IWatcher(Oracle<ADDRESS, FLAGS> *wp_ref);
   ~IWatcher();
   
   int  general_fault    (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int  watch_fault      (ADDRESS start_addr, ADDRESS end_addr);
   int  read_fault       (ADDRESS start_addr, ADDRESS end_addr);
   int  write_fault      (ADDRESS start_addr, ADDRESS end_addr);
   
   int  add_watchpoint   (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int  rm_watchpoint    (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address             (ADDRESS target_addr);
private:
   Oracle<ADDRESS, FLAGS>              *wp;
   
   L1Cache<ADDRESS>                    l1_data;
   L2Cache<ADDRESS>                    l2_data;
   PageTable1_single<ADDRESS, FLAGS>   vm_system;
};

#include "iwatcher.cpp"
#endif /*IWATCHER_H_*/

/*
pseudocode for iwatcher
general fault:
   if (hit in l1) : update lru in l1
   if (hit in l2) : push into l1, update lru
   if (hit in victim cache) : push into l1 and l2, remove in victim cache
   if (miss) : check for availability in VM, if available => bring in required cache line (to act like a real cache)
                                             if unavailable => fetch only watched cache lines, then mark as avaiable in VM.
   if (l2 kickout) : push it in victim cache, mark the page as unavailable if necessary (i.e. check it before set it)
wp_operation:
   if (hit) : update lru
   add_watchpoint : mark the page as unavailable
   rm_watchpoint : if (originally available) --- do nothing
                   else check if the page should be available now

conclusion:
1. only write in l1 and l2 when read miss (no write allocate)
2. only kickouts from l2 are written in victim cache
3. l2 kickouts may cause VM availability to change (l1 and victim cache kickouts doesn't matter)
4. wp_operation only modifies VM availability and lru
*/
