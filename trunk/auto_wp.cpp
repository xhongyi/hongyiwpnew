#include "auto_wp.h"
#include <iostream>

using namespace std;

/*
 * Constructor
 * Initialize the wp system as the whole memory with clean tag
 * As the watchpoint is construct to record memory, the ADDRESS
 * should be unsigned type(which it is in pin.) So we cover the
 * whole memory by setting the end_addr to be max = -1(b1111....1111)
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint() {
	watchpoint_t<ADDRESS, FLAGS> temp;
	temp.start_addr = 0;
	temp.end_addr = -1;
	temp.flags = 0;
	wp.push_back(temp);
}

template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::~WatchPoint() {
}
/*
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::add_watch(ADDRESS start_addr, ADDRESS end_addr) {

}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::add_read(ADDRESS start_addr, ADDRESS end_addr) {

}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::add_write(ADDRESS start_addr, ADDRESS end_addr) {

}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::rm_watch(ADDRESS start_addr, ADDRESS end_addr) {

}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::rm_read(ADDRESS start_addr, ADDRESS end_addr) {

}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::rm_write(ADDRESS start_addr, ADDRESS end_addr) {

}
*/
/*
 * Below 3 wp fault detector all utilize the general_fault() function,
 * with different detection flags
 */
template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
	return general_fault(start_addr, end_addr, WA_READ|WA_WRITE);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
	return general_fault(start_addr, end_addr, WA_READ);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
	return general_fault(start_addr, end_addr, WA_WRITE);
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::watch_print() {
	cout << "There are " << wp.size() << " watchpoints" << endl;
	for (unsigned int i = 0; i < wp.size() ; i++) {
		cout << "This is the " << i << "th watchpoint" << endl;
		cout << "start_addr = " << wp[i].start_addr << endl;
		cout << "end_addr = " << wp[i].end_addr << endl;
		if (wp[i].flags & WA_READ)
			cout << "R";
		if (wp[i].flags & WA_WRITE)
			cout << "W";
		cout << endl;
	}
	return;
}
/*
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {

}

template<class ADDRESS, class FLAGS>
void atchPoint::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {

}
*/
template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	wp_iter = search_address (start_addr, wp);
	while (wp_iter != wp.end() && end_addr > wp_iter->end_addr) {
		if (wp_iter->flags & target_flags)
			return true;
		wp_iter++;
	}
	return ( (wp_iter != wp.end() ) && (wp_iter->flags & target_flags) );
}

/*
 * Binary search. Return the iter of the range contain the start_addr.
 * We also assume that the wp deque is sorted.
 */
template<class ADDRESS, class FLAGS>
typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
	search_address (ADDRESS start_addr, deque<watchpoint_t<ADDRESS, FLAGS> > &wp) {
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator beg_iter;
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator mid_iter;
	typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator end_iter;
	beg_iter = wp.begin();
	end_iter = wp.end();
	while (beg_iter < end_iter - 1) {
		mid_iter = beg_iter + (end_iter - beg_iter) / 2;
		if (start_addr <= mid_iter->start_addr)
			end_iter = mid_iter;
		else
			beg_iter = mid_iter;
	}
	/*
	 * The iteration will stop at the point where beg + 1 = end
	 * So we need to compare further more to decide which one contains start_addr
	 */
	if (start_addr <= beg_iter->end_addr)
		return beg_iter;
	else
		return end_iter;
}
