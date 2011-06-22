#ifndef RANGE_CACHE_H_
#define RANGE_CACHE_H_
#include <deque>
#include "cache_algo.h"
#include "oracle_wp.h"
#include "wp_data_struct.h"

#define     CACHE_SIZE     64
#define     CACHE_MISS     0
#define     CACHE_HIT      1

#define     DIRTY          16

template<class ADDRESS, class FLAGS>
class RangeCache {
public:
   RangeCache(Oracle<ADDRESS, FLAGS> *wp_ref);
   RangeCache();
   ~RangeCache();
   // for all ranges referred to, update lru if found in rc; load from backing store if not found.
   int   general_fault  (ADDRESS start_addr, ADDRESS end_addr, bool dirty = false);
   int   watch_fault (ADDRESS start_addr, ADDRESS end_addr);
   int   read_fault  (ADDRESS start_addr, ADDRESS end_addr);
   int   write_fault (ADDRESS start_addr, ADDRESS end_addr);
   // actually updating all the entries referred by this range
   int   wp_operation   (ADDRESS start_addr, ADDRESS end_addr);
   int   add_watchpoint (ADDRESS start_addr, ADDRESS end_addr);
   int   rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr);
   // for getting statistics
   void  get_stats      (long long &kickout_out, long long &kickout_dirty_out, long long &complex_updates_out);
   // print for debugging
   void watch_print     (ostream &output = cout);
private:
   // data structure used in range cache
   deque< watchpoint_t<ADDRESS, FLAGS> >  rc_data;       // cache data
   Oracle<ADDRESS, FLAGS>*                oracle_wp;     // storing a pointer to its corresponding oracle
   long long                              kickout_dirty, 
                                          kickout, 
                                          complex_updates;
   // private functions used in range cache
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address    (ADDRESS target_addr);           // search for target addr in rc, return rc_data.end() if miss
   void rm_range(ADDRESS start_addr, ADDRESS end_addr);  // remove all entries within the range
   bool cache_overflow();
   void cache_kickout();   // kickout one lru entry at a time
};

#include "range_cache.cpp"
#endif /*RANGE_CACHE_H_*/
