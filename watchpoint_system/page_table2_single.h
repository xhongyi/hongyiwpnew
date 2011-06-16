/*
 * page_table2_single.h
 *    two level page_table for a single thread
 *  Created on: May 17, 2011
 *      Author: luoyixin
 */
 
#ifndef PAGE_TABLE2_SINGLE_H_
#define PAGE_TABLE2_SINGLE_H_

#define ALL_WATCHED           0
#define ALL_UNWATCHED         1
#define SUPERPAGE_WATCHED     2
#define SUPERPAGE_UNWATCHED   3
#define PAGETABLE_WATCHED     4
#define PAGETABLE_UNWATCHED   5

#include "virtual_wp.h"
#include "page_table1_single.h"

#define SUPERPAGE_NUM_LENGTH              10UL
#define SECOND_LEVEL_PAGE_NUM_LENGTH      (VIRTUAL_ADDRESS_LENGTH-PAGE_OFFSET_LENGTH-SUPERPAGE_NUM_LENGTH)  // 10
#define SUPERPAGE_OFFSET_LENGTH           (VIRTUAL_ADDRESS_LENGTH-SUPERPAGE_NUM_LENGTH)                     // 22
#define SUPER_PAGE_BIT_MAP_NUMBER         (1<<(SUPERPAGE_NUM_LENGTH-BIT_MAP_OFFSET_LENGTH))

template<class ADDRESS, class FLAGS>
class PageTable2_single : public Virtual_wp<ADDRESS, FLAGS> {
public:
   PageTable2_single(Virtual_wp<ADDRESS, FLAGS> *pt1_ref);
   PageTable2_single();
   ~PageTable2_single();
   
   int   general_fault  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
   int   watch_fault    (ADDRESS start_addr, ADDRESS end_addr);
   int   read_fault     (ADDRESS start_addr, ADDRESS end_addr);
   int   write_fault    (ADDRESS start_addr, ADDRESS end_addr);
   
   int   add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
   int   rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0);
   
   bool  check_unity    (ADDRESS superpage_number, bool watched);
   void  watch_print    (ostream &output = cout);
private:
   /*
    * initialized when constructing
    * used for checking each page's state
    */
   Virtual_wp<ADDRESS, FLAGS> *pt1;
   /*
    * two bits for all pages
    * watched(10), unwatched(01) or missed(00)
    */
   bool all_watched;
   bool all_unwatched;
   /*
    * two bits for each 2nd_level_page
    * watched(10), unwatched(01) or missed(00)
    */
   unsigned char superpage_watched[SUPER_PAGE_BIT_MAP_NUMBER];
   unsigned char superpage_unwatched[SUPER_PAGE_BIT_MAP_NUMBER];
   
   //unsigned char bit_map[BIT_MAP_NUMBER];
};

#include "page_table2_single.cpp"
#endif
