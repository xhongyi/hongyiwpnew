/*
 * page_table1_multi.h
 *
 *  Created on: May 26, 2011
 *      Author: luoyixin
 */

#ifndef PAGE_TABLE1_MULTI_H_
#define PAGE_TABLE1_MULTI_H_

#include <stdint.h>
#include <map>
#include "virtual_wp.h"
#include "page_table1_single.h"
using namespace std;

template<class ADDRESS, class FLAGS>
class PageTable1_multi : public Virtual_wp<ADDRESS, FLAGS> {
public:
   PageTable1_multi();
   ~PageTable1_multi();
   // Threading Calls
   void     start_thread   (int32_t thread_id, PageTable1_single<ADDRESS, FLAGS>* pt_in);
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
   /*
    * initialized when constructing
    * used for checking each page's state when rm_watchpoint is called
    */
   map<int32_t, PageTable1_single<ADDRESS, FLAGS>* >        page_table_wp;       // only a copy of the pointers, DO NOT own them
   // iterator
   typename map<int32_t, PageTable1_single<ADDRESS, FLAGS>* >::iterator page_table_wp_iter;
   /*
    * one bit for each page
    * keeping track of whether this page is watched or not
    * the software will keep what kind of watchpoint it is
    */
   unsigned char bit_map[BIT_MAP_NUMBER];
};

#include "page_table1_multi.cpp"
#endif /* PAGE_TABLE1_MULTI_H_ */
