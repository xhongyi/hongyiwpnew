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

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	wp_operation(start_addr, end_addr, target_flags, &flag_include, &flag_union);
}


template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	wp_operation(start_addr, end_addr, target_flags, &flag_exclude, &flag_diff);
}

/*
 * The idea of adding wp is by splitting the procedure into 3 parts:
 * The Begin Part as it would search if we need to split or merge.
 * The Iterating Part as it would iterate through and do merge if
 * necessary.
 * The End Part which is also similar to the Begin Part, splitting
 * and merging if necessary.
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::wp_operation(ADDRESS start_addr, ADDRESS end_addr,
		FLAGS target_flags, bool (*flag_test)(FLAGS &x, FLAGS &y),
		FLAGS (*flag_op)(FLAGS &x, FLAGS &y) ) {
	/*
	 * insert_t is used for keeping temp data before being inserted into wp.
	 */
	watchpoint_t<ADDRESS, FLAGS> insert_t;
	/*
	 * The first search must fall into a range either tagged or not.
	 */
	wp_iter = search_address (start_addr, wp);
	/*
	 * Special case: If the target range is so small that it falls
	 * into 1 single range, then we do not need to go through the 3
	 * Parts as there is no wp to iterating.
	 * Also we need to check both begin and end splitting and merge
	 */
	if (wp_iter->end_addr >= end_addr) {
		/*
		 * We only modify wp system if there is some change. Otherwise, we return
		 */
		if (!flag_test(wp_iter->flags, target_flags) ) {
			/*
			 * This case we need to check merge.
			 * Note we check if start_addr == 0. If it's 0 then there is no "previous wp"
			 * First we check the front condition.
			 */
			if (wp_iter->start_addr == start_addr) {
				/*
				 * If start_addr == 0 then won't merge
				 */
				if (start_addr != 0) {
					pre_iter = wp_iter - 1;
					/*
					 * Merge
					 */
					if (pre_iter->flags == flag_op(wp_iter->flags, target_flags) ) {
						wp_iter->start_addr = pre_iter->start_addr;
						wp_iter = wp.erase(pre_iter); //Erase pre_iter and restore wp_iter.
					}
					//We can't update the flags for now. We must wait until we have
					//determined wether merge or split at the end_addr.
				}
			}
			/*
			 * If "start_addr"s are not equal then me are facing a split.
			 * We split by change wp_iter's start_addr and add the "split" part
			 * before wp_iter.
			 */
			else {
				insert_t.start_addr = wp_iter->start_addr;
				insert_t.end_addr = start_addr - 1;
				insert_t.flags = wp_iter->flags;
				wp_iter->start_addr = start_addr;
				wp_iter = wp.insert(wp_iter, insert_t) + 1; //Insert and Restore wp_iter position.
			}

			/*
			 * Then we check for the end.
			 * Also merge or split as we did for front.
			 */
			aft_iter = wp_iter + 1;
			if (wp_iter->end_addr == end_addr) {
				/*
				 * If end_addr == -1 then won't merge
				 */
				if (end_addr != (ADDRESS)-1) {
					/*
					 * Merge
					 */
					if (aft_iter->flags == flag_op(wp_iter->flags, target_flags) ) {
						wp_iter->end_addr = aft_iter->end_addr;
						wp.erase(aft_iter); //No need to restore wp_iter as we gonna return.
					}
				}
				// Whether merge or not, change the flags.
				wp_iter->flags = flag_op (wp_iter->flags, target_flags);
			}
			/*
			 * Split
			 */
			else {
				insert_t.start_addr = wp_iter->start_addr;
				insert_t.end_addr = end_addr;
				insert_t.flags = flag_op(wp_iter->flags, target_flags);
				wp_iter->start_addr = end_addr + 1;
				wp.insert(wp_iter, insert_t); //No need to restore as we are done.
			}
		}
		return;
	}

	/*
	 * Begin part. We need to decide whether merge or split again.
	 * The difference between the above code is we now need to change wp_iter's
	 * flags immediately as we at most split 1 range into 2, while in above code
	 * we may split 1 into 3(beg, mid and end). And we need to increment wp_iter
	 * as well.
	 */
	/*
	 * We only change wp if the target_flags is excluded.
	 */
	if (!flag_test(wp_iter->flags, target_flags) ) {
		if (wp_iter->start_addr == start_addr) {
			/*
			 * If start_addr == 0 then won't merge
			 */
			if (start_addr != 0) {
				pre_iter = wp_iter - 1;
				/*
				 * Merge
				 */
				if (pre_iter->flags == flag_op(wp_iter->flags, target_flags) ) {
					wp_iter->start_addr = pre_iter->start_addr;
					wp_iter->flags = flag_op(wp_iter->flags, target_flags);
					wp_iter = wp.erase(pre_iter); //erase and restore wp_iter.
				}
			}
			/*
			 * merge or not, we still need to change flags and increment wp_iter.
			 */
			wp_iter->flags = flag_op(wp_iter->flags, target_flags);
			wp_iter++; //Increment wp_iter.
		}
		/*
		 * Split
		 */
		else {
			insert_t.start_addr = wp_iter->start_addr;
			insert_t.end_addr = start_addr - 1;
			insert_t.flags = wp_iter->flags;
			wp_iter->start_addr = start_addr;
			wp_iter->flags = flag_op(wp_iter->flags, target_flags);
			wp_iter = wp.insert(wp_iter, insert_t) + 2; //Insert and increment wp_iter.
		}
	}
	else
		wp_iter++; //Increment wp_iter.

	/*
	 * Iterating part
	 */
	while (wp_iter->end_addr < end_addr) {
		pre_iter = wp_iter - 1;
		/*
		 * Union the flags.
		 */
		wp_iter->flags = flag_op(wp_iter->flags, target_flags);
		/*
		 * Check merge. Merge by enlarge the pre_iter and erase wp_iter.
		 */
		if (pre_iter->flags == wp_iter->flags) {
			pre_iter->end_addr = wp_iter->end_addr;
			wp_iter = wp.erase(wp_iter); //Erase wp_iter and increment wp_iter.
		}
		else
			wp_iter++; //Increment wp_iter.
	}
	/*
	 * Ending part. Also we need to check merge or split. Besides, we also need
	 * to check if we also will merge with the range before it.
	 */
	/*
	 * We only change wp if the target_flags is excluded.
	 */
	if (!flag_test(wp_iter->flags, target_flags) ) {
		pre_iter = wp_iter - 1;
		aft_iter = wp_iter + 1;
		if (wp_iter->end_addr == end_addr) {
			/*
			 * First check if it should merge with the wp before it.
			 */
			wp_iter->flags = flag_op(wp_iter->flags, target_flags);
			if (wp_iter->flags == pre_iter->flags) {
				pre_iter->end_addr = wp_iter->end_addr;
				wp.erase(wp_iter);
				wp_iter = pre_iter; //restore wp_iter
				aft_iter = wp_iter + 1; //restore aft_iter
			}
			/*
			 * If end_addr = -1 then we won't merge.
			 */
			if (end_addr != (ADDRESS)-1) {
				/*
				 * Merge
				 */
				if (aft_iter->flags == flag_op(wp_iter->flags, target_flags) ) {
					wp_iter->end_addr = aft_iter->end_addr;
					wp.erase(aft_iter); //erase and increment wp_iter.
				}
			}
		}
		/*
		 * Split
		 */
		else {
			/*
			 * Check if wp_iter will merge with the pre_iter.
			 */
			if ( flag_op(wp_iter->flags, target_flags) == pre_iter->flags) {
				wp_iter->start_addr = end_addr + 1;
				pre_iter->end_addr = end_addr;
			}
			/*
			 * If not, then we need to split by insertion.
			 */
			else {
				insert_t.start_addr = wp_iter->start_addr;
				insert_t.end_addr = end_addr;
				insert_t.flags = flag_op(wp_iter->flags, target_flags);
				wp_iter->start_addr = end_addr + 1;
				wp.insert(wp_iter, insert_t); //Insert, no need to restore wp_iter
			}
		}
	}
	/*
	 * Although the flags are already included. We still need to check if we need
	 * to merge with the wp before it.
	 */
	else {
		pre_iter = wp_iter - 1;
		if (wp_iter->flags == pre_iter->flags) {
			pre_iter->end_addr = wp_iter->end_addr;
			wp.erase(wp_iter);
		}
	}

	return;
}

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
	WatchPoint<ADDRESS, FLAGS>::search_address (ADDRESS start_addr, deque<watchpoint_t<ADDRESS, FLAGS> > &wp) {
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

/*
 * Returns true if all the target_flags are included by container_flags
 */
template<class FLAGS>
bool flag_include(FLAGS container_flags, FLAGS target_flags) {
	return ( (target_flags & container_flags) == target_flags);
}

template<class FLAGS>
bool flag_exclude(FLAGS container_flags, FLAGS target_flags) {
	return ( (target_flags & ~container_flags) == target_flags);
}

template<class FLAGS>
FLAGS flag_union (FLAGS &x, FLAGS &y) {
	return (x | y);
}

/*
 * Returns the flags of FLAGS (x - y)
 */
template<class FLAGS>
FLAGS flag_diff (FLAGS &x, FLAGS &y) {
	return (x & ~y);
}
