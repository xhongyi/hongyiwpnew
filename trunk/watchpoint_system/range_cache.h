#ifndef RANGE_CACHE_H_
#define RANGE_CACHE_H_
#include <deque>
#include <algorithm>
#include "virtual_wp.h"
#include "wp_data_struct.h"

#define     CACHE_SIZE     64
#define     OCBM_LEN       10

#define     DIRTY          16

template<class ADDRESS, class FLAGS>
class RangeCache {
public:
   RangeCache(Virtual_wp<ADDRESS, FLAGS> *wp_ref, bool ocbm_in = false);
   RangeCache();
   ~RangeCache();
   // for all ranges referred to, update lru if found in rc; load from backing store if not found.
   int   general_fault  (ADDRESS start_addr, ADDRESS end_addr);
   int   watch_fault (ADDRESS start_addr, ADDRESS end_addr);
   int   read_fault  (ADDRESS start_addr, ADDRESS end_addr);
   int   write_fault (ADDRESS start_addr, ADDRESS end_addr);
   // actually updating all the entries referred by this range
   int   wp_operation   (ADDRESS start_addr, ADDRESS end_addr, bool is_update);
   int   add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, bool is_update);
   int   rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, bool is_update);
   // for getting statistics
   void  get_stats      (long long &kickout_out, long long &kickout_dirty_out, long long &complex_updates_out);
   // print for debugging
   void watch_print     (ostream &output = cout);
   // only for range_cache with ocbm function
   void check_ocbm      (ADDRESS start_addr, ADDRESS end_addr);
private:
   bool ocbm;
   // data structure used in range cache
   deque< watchpoint_t<ADDRESS, FLAGS> >  rc_data;       // cache data
   Virtual_wp<ADDRESS, FLAGS>*            oracle_wp;     // storing a pointer to its corresponding oracle
   long long                              kickout_dirty, 
                                          kickout, 
                                          complex_updates;
   // private functions used in range cache
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address    (ADDRESS target_addr);           // search for target addr in rc, return rc_data.end() if miss
   int rm_range(ADDRESS start_addr, ADDRESS end_addr);  // remove all entries within the range
   bool cache_overflow();
   void cache_kickout();   // kickout one lru entry at a time
};

#include "range_cache.cpp"
#endif /*RANGE_CACHE_H_*/
