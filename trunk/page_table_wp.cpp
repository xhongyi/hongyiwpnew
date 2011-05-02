#include "page_table_wp.h"
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
WatchPoint_PT<ADDRESS, FLAGS>::WatchPoint_PT() {
	for (int i=0;i<BIT_MAP_NUMBER;i++)
		bit_map[i] = 0;
}

template<class ADDRESS, class FLAGS>
WatchPoint_PT<ADDRESS, FLAGS>::~WatchPoint_PT() {
}

template<class ADDRESS, class FLAGS>
void WatchPoint_PT<ADDRESS, FLAGS>::watch_print() {
	cout <<"Start printing watchpoints..."<<endl;
	for (ADDRESS i=0;i<PAGE_NUMBER;i++) {
		if (bit_map[i>>3] & (1<<(i&0x7)) )
			cout <<"page number "<<i<<" is being watched."<<endl;
	}
	return;
}

template<class ADDRESS, class FLAGS>
bool WatchPoint_PT<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {
		if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )
			return true;
	}
	return false;
}

template<class ADDRESS, class FLAGS>
unsigned int WatchPoint_PT<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	unsigned int num_changes = 0;
	if (target_flags) {
		ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
		ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
		for (ADDRESS i=page_number_start;i<=page_number_end;i++) {
			if (!(bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))
				num_changes++;
			bit_map[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));
		}
	}
	return num_changes;
}

template<class ADDRESS, class FLAGS>
unsigned int WatchPoint_PT<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, WatchPoint<ADDRESS, FLAGS> &wp) {
	unsigned int num_changes = 0;
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {
		if (!check_page_fault(i, wp)) {
			if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )
				num_changes++;
			bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
		}
	}
	return num_changes;
}

template<class ADDRESS, class FLAGS>
bool WatchPoint_PT<ADDRESS, FLAGS>::check_page_fault(ADDRESS page_number, WatchPoint<ADDRESS, FLAGS> &wp) {
	ADDRESS start_addr = (page_number<<PAGE_OFFSET_LENGTH);
	ADDRESS end_addr = ((page_number+1)<<PAGE_OFFSET_LENGTH)-1;
	return wp.watch_fault(start_addr, end_addr);
}


