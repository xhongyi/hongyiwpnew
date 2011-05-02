/*
 * page_table_wp.h
 *
 *  Created on: May 1, 2011
 *      Author: luoyixin
 */

#ifndef PAGE_TABLE_WP_H_
#define PAGE_TABLE_WP_H_

#define	VIRTUAL_ADDRESS_LENGTH	32
#define	PAGE_OFFSET_LENGTH		12
#define	PAGE_NUMBER				(1<<12)
#define	BIT_MAP_OFFSET_LENGTH	3
#define	BIT_MAP_NUMBER_LENGTH	17
#define	BIT_MAP_NUMBER			(1<<17)

#include "auto_wp.h"

template<class ADDRESS, class FLAGS>
class WatchPoint_PT:public WatchPoint<ADDRESS, FLAGS> {
public:
	WatchPoint_PT();
	~WatchPoint_PT();
	
	//bool	general_fault	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	bool	watch_fault		(ADDRESS start_addr, ADDRESS end_addr);
	//bool	read_fault		(ADDRESS start_addr, ADDRESS end_addr);
	//bool	write_fault		(ADDRESS start_addr, ADDRESS end_addr);
	
	//typename std::deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
	//		search_address	(ADDRESS start_addr, std::deque<watchpoint_t<ADDRESS, FLAGS> > &wp);

	void	watch_print();
	
	unsigned int	add_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	unsigned int	rm_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, WatchPoint<ADDRESS, FLAGS> &wp);

//private:
	//called by rm_watchpoint to check the whole page in the WatchPoint system
	bool	check_page_fault(ADDRESS page_number, WatchPoint<ADDRESS, FLAGS> &wp);

	unsigned char bit_map[BIT_MAP_NUMBER];
};

#include "page_table_wp.cpp"
#endif /* PAGE_TABLE_WP_H_ */
