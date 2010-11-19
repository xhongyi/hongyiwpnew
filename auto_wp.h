/*
 * auto_wp.h
 *
 *  Created on: Nov 18, 2010
 *      Author: xhongyi
 */

#ifndef AUTO_WP_H_
#define AUTO_WP_H_

#include <deque>

#define	WA_READ			1
#define	WA_WRITE		2

template<class ADDRESS, class FLAGS>
struct watchpoint_t {
	ADDRESS		start_addr;
	ADDRESS		end_addr;
	FLAGS		flags;
};

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

	void	watch_print();
//private:
/*
	void	add_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
	void	rm_watchpoint	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
*/
	bool	general_fault	(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);

	std::deque< watchpoint_t<ADDRESS, FLAGS> > wp;
	typename std::deque< watchpoint_t<ADDRESS, FLAGS> >::iterator wp_iter;
};

#include "auto_wp.cpp"

template<class ADDRESS, class FLAGS>
typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
	search_address (ADDRESS start_addr, deque<watchpoint_t<ADDRESS, FLAGS> > &wp);
#endif /* AUTO_WP_H_ */
