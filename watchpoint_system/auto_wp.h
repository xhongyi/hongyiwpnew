/*
 * auto_wp.h
 *
 *  Created on: Nov 18, 2010
 *      Author: xhongyi
 */

#ifndef _AUTO_WP_H_
#define _AUTO_WP_H_

#include <deque>
#include "wp_data_struct.h"

template<class ADDRESS, class FLAGS>
class WatchPoint {
public:
	WatchPoint();
	~WatchPoint();
/*
	void	add_watch	(ADDRESS start_addr, ADDRESS end_addr);
	void	add_read	(ADDRESS start_addr, ADDRESS end_addr);
	void	add_write	(ADDRESS start_addr, ADDRESS end_addr);

	void	rm_watch	(ADDRESS start_addr, ADDRESS end_addr);
	void	rm_read		(ADDRESS start_addr, ADDRESS end_addr);
	void	rm_write	(ADDRESS start_addr, ADDRESS end_addr);
*/
	bool	watch_fault	(ADDRESS start_addr, ADDRESS end_addr);
	bool	read_fault		(ADDRESS start_addr, ADDRESS end_addr);
	bool	write_fault	(ADDRESS start_addr, ADDRESS end_addr);
	
	typename std::deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
		search_address (ADDRESS start_addr, std::deque<watchpoint_t<ADDRESS, FLAGS> > &wp);

	void	watch_print();
//private:

	void	wp_operation	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags,
			 bool (*flag_test)(FLAGS &x, FLAGS &y), FLAGS (*flag_op)(FLAGS &x, FLAGS &y) );
	void	add_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void	rm_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	bool	general_fault	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);

	/*
	 * wp		is the container to hold all the ranges for watchpoint structure.
	 * wp_iter	is the iterator used in both add_watchpoint and rm_watchpoint.
	 * pre_iter	is used for getting the node right before wp_iter.
	 * aft_iter	is used for getting the node right after wp_iter.
	 *
	 * All these data are declared here is to save time by avoiding creating
	 * and destroying them every time add_watchpoint and rm_watchpoint is called.
	 */
	std::deque< watchpoint_t<ADDRESS, FLAGS> > wp;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator wp_iter;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator pre_iter;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator aft_iter;
};
/*
template<class ADDRESS, class FLAGS>
typename std::deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
	search_address (ADDRESS start_addr, std::deque<watchpoint_t<ADDRESS, FLAGS> > &wp);
*/

template<class FLAGS>
bool flag_include(FLAGS container_flags, FLAGS target_flags);

template<class FLAGS>
bool flag_exclude(FLAGS container_flags, FLAGS target_flags);

template<class FLAGS>
FLAGS flag_union (FLAGS &x, FLAGS &y);

template<class FLAGS>
FLAGS flag_diff (FLAGS &x, FLAGS &y);

#include "auto_wp.cpp"
#endif /* AUTO_WP_H_ */
