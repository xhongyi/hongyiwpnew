/*
 * auto_wp.h
 *
 *  Created on: Nov 18, 2010
 *      Author: xhongyi
 */

#ifndef AUTO_WP_H_
#define AUTO_WP_H_

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

	void	add_watch (ADDRESS start_addr, ADDRESS end_addr);
};

#endif /* AUTO_WP_H_ */
