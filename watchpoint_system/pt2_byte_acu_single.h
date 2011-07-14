/*
 * pt2_byte_acu_single.h
 *
 *  Created on: May 27, 2011
 *      Author: luoyixin
 */
#ifndef PT2_BYTE_ACU_SINGLE_H_
#define PT2_BYTE_ACU_SINGLE_H_

#define SUPER_PAGE_BIT_MAP_NUMBER         (1<<(SUPERPAGE_NUM_LENGTH-BIT_MAP_OFFSET_LENGTH))

#define BITMAP_WATCHED     6
#define BITMAP_UNWATCHED   7

#include <bitset>
#include "plb.h"
#include "virtual_wp.h"
#include "page_table2_single.h"
using namespace std;

template<class ADDRESS, class FLAGS>
class PT2_byte_acu_single : public Virtual_wp<ADDRESS, FLAGS> {
public:
   PT2_byte_acu_single(Virtual_wp<ADDRESS, FLAGS> *wp_ref);
   PT2_byte_acu_single();
   ~PT2_byte_acu_single();
   /*
    * this function tells all pages covered by this range is watched or not
    */
   int     general_fault  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int     watch_fault    (ADDRESS start_addr, ADDRESS end_addr);
   int     read_fault     (ADDRESS start_addr, ADDRESS end_addr);
   int     write_fault    (ADDRESS start_addr, ADDRESS end_addr);
   /*
    * returns the number of changes it does on bit_map
    * for counting the number of changes: changes = add or rm;
    */
   int      add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int      rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   
   bool     check_page_level_consistency        (ADDRESS page_number,      FLAGS target_flags, bool watched);
   bool     check_superpage_level_consistency   (ADDRESS superpage_number, FLAGS target_flags, bool watched);
   bool     check_seg_reg_level_consistency     (                          FLAGS target_flags, bool watched);
   bool     check_page_level_consistency        (ADDRESS page_number);
   bool     check_superpage_level_consistency   (ADDRESS superpage_number);
   bool     check_seg_reg_level_consistency     ();
   // this is just an empty function
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address             (ADDRESS target_addr)
      {return wp->search_address(target_addr);}
   // extra statistics
   long long plb_misses;
private:
   /*
    * initialized when constructing
    * used for checking each page's state when rm_watchpoint is called
    */
   Virtual_wp<ADDRESS, FLAGS> *wp;
   /*
    * two bits for all pages (in segmentation register)
    * watched(10), unwatched(01) or missed(00)
    */
   bool seg_reg_read_watched;
   bool seg_reg_write_watched;
   bool seg_reg_unknown;
   /*
    * two bits for each 2nd_level_page
    */
   bitset<SUPERPAGE_NUM> superpage_read_watched;
   bitset<SUPERPAGE_NUM> superpage_write_watched;
   bitset<SUPERPAGE_NUM> superpage_unknown;
   /*
    * two bits for each page
    */
   bitset<PAGE_NUMBER> pt_read_watched;
   bitset<PAGE_NUMBER> pt_write_watched;
   bitset<PAGE_NUMBER> pt_unknown;
   // protection lookaside buffer
   PLB<ADDRESS>  plb;
};

#include "pt2_byte_acu_single.cpp"
#endif /* PT2_BYTE_ACU_SINGLE_H_ */
