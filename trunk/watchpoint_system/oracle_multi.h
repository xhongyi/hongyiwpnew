/*
 * oracle_multi.h
 *
 *  Created on: Jun 13, 2011
 *      Author: luoyixin
 */
#ifndef ORACLE_MULTI_H_
#define ORACLE_MULTI_H_

#include <stdint.h>
#include <map>
#include "oracle_wp.h"
using namespace std;

template<class ADDRESS, class FLAGS>
class Oracle_multi : public Virtual_wp<ADDRESS, FLAGS> {
public:
   Oracle_multi();
   ~Oracle_multi();
   // Threading Calls
   void     start_thread   (int32_t thread_id, Oracle<ADDRESS, FLAGS>* oracle_in);
   void     end_thread     (int32_t thread_id);
   /*
    * this function tells all pages covered by this range is watched or not
    */
   int      general_fault  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
   int      watch_fault    (ADDRESS start_addr, ADDRESS end_addr);
   int      read_fault     (ADDRESS start_addr, ADDRESS end_addr);
   int      write_fault    (ADDRESS start_addr, ADDRESS end_addr);
   /*
    * returns the number of changes it does on bit_map
    * for counting the number of changes: changes = add + rm;
    */
   int      add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
   int      rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
private:
   Oracle<ADDRESS, FLAGS> wp;
   map<int32_t, Oracle<ADDRESS, FLAGS>* >                      oracle_wp;
   typename map<int32_t, Oracle<ADDRESS, FLAGS>* >::iterator   oracle_wp_iter;
};

#include "oracle_multi.cpp"
#endif
