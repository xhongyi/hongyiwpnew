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

#include <bitset>
#include "virtual_wp.h"
#include "page_table1_single.h"
using namespace std;

#define SUPERPAGE_NUM_LENGTH              10UL
#define SUPERPAGE_NUM                     (1<<SUPERPAGE_NUM_LENGTH)
#define SECOND_LEVEL_PAGE_NUM_LENGTH      (VIRTUAL_ADDRESS_LENGTH-PAGE_OFFSET_LENGTH-SUPERPAGE_NUM_LENGTH)  // 10
#define SUPERPAGE_SIZE                    (1<<SECOND_LEVEL_PAGE_NUM_LENGTH)
#define SUPERPAGE_OFFSET_LENGTH           (VIRTUAL_ADDRESS_LENGTH-SUPERPAGE_NUM_LENGTH)                     // 22

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
   // this is just an empty function
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address             (ADDRESS target_addr)
      {return pt1->search_address(target_addr);}
private:
   /*
    * initialized when constructing
    * used for checking each page's state
    */
   Virtual_wp<ADDRESS, FLAGS>        *pt1;
   /*
    * two bits for all pages
    * watched(10), unwatched(01) or missed(00)
    */
   bool all_watched;
   bool all_unwatched;
   /*
    * two bits for each superpage
    * watched(10), unwatched(01) or missed(00)
    */
   bitset<SUPERPAGE_NUM>  superpage_watched;
   bitset<SUPERPAGE_NUM>  superpage_unwatched;
   // but only one bit for 2nd_level_pagetable
   bitset<PAGE_NUMBER> pagetable_watched;
   // private function called by add/rm_watchpoint, when some superpage is set to unknown. (use as a pagetable history)
   int set_unknown(ADDRESS superpage_number);
};

#include "page_table2_single.cpp"
#endif
