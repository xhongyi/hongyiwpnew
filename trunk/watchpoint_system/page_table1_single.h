/*
 * page_table1_single.h
 *
 *  Created on: May 1, 2011
 *      Author: luoyixin
 */

#ifndef PAGE_TABLE1_SINGLE_H_
#define PAGE_TABLE1_SINGLE_H_

#define  VIRTUAL_ADDRESS_LENGTH  32
#define  PAGE_OFFSET_LENGTH      12
#define  PAGE_SIZE               (1<<PAGE_OFFSET_LENGTH)       //page size is 4 KB
#define  PAGE_NUMBER             (1<<(VIRTUAL_ADDRESS_LENGTH-PAGE_OFFSET_LENGTH))
#define  BIT_MAP_OFFSET_LENGTH   3        //we store one bit for each page in a bit map, 
                                          //which is byte addressable
#define  BIT_MAP_NUMBER_LENGTH   (VIRTUAL_ADDRESS_LENGTH - PAGE_OFFSET_LENGTH - BIT_MAP_OFFSET_LENGTH)
#define  BIT_MAP_NUMBER          (1<<BIT_MAP_NUMBER_LENGTH) //needs 128 KB to store these bits

#include "oracle_wp.h"

template<class ADDRESS, class FLAGS>
class PageTable1_single {
public:
	PageTable1_single(Oracle<ADDRESS, FLAGS> &wp_ref);
	PageTable1_single();
	~PageTable1_single();
	
	/*
	 * this function tells all pages covered by this range is watched or not
	 */
	bool     watch_fault    (ADDRESS start_addr, ADDRESS end_addr);
	
	/*
	 * we don't need these functions because the page table do not keep 
	 * what kind of watchpoint it is having
	 */
	
	//bool   read_fault  (ADDRESS start_addr, ADDRESS end_addr);
	//bool   write_fault (ADDRESS start_addr, ADDRESS end_addr);
	
	void     watch_print (ostream &output = cout);
	
	/*
	 * returns the number of changes it does on bit_map
	 * for set watchpoint: just call add_watchpoint
	 * for update watchpoint: needs to call rm_watchpoint on all ranges and then add_watchpoint to overwrite
	 * for counting the number of changes: update = abs(add - rm);
	 *
	 * add_watchpoint needs target_flags just to check if it is none flag or not
	 */
	void     add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void     rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	/*
	unsigned int   add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	unsigned int   rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr);
	*/

//private:
	/*
	 * initialized when constructing
	 * used for checking each page's state when rm_watchpoint is called
	 */
	Oracle<ADDRESS, FLAGS> *wp;
	
	/*
	 * one bit for each page
	 * keeping track of whether this page is watched or not
	 * the software will keep what kind of watchpoint it is
	 */
	unsigned char bit_map[BIT_MAP_NUMBER];
};

#include "page_table1_single.cpp"
#endif /* PAGE_TABLE1_SINGLE_H_ */
