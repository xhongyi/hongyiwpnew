#include "cache_algo.h"
#include <iostream>

using namespace std;

template <class ADDRESS>
CacheAlgo<ADDRESS>::CacheAlgo() {
}

template <class ADDRESS>
CacheAlgo<ADDRESS>::~CacheAlgo() {
}

template <class ADDRESS>
int CacheAlgo<ADDRESS>::modify(ADDRESS start_addr, ADDRESS end_addr, ADDRESS new_start, ADDRESS new_end) {
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = search_iter(start_addr, end_addr);
	if (iter == ranges.end() )
		return -1;
	else {
		iter->start_addr = new_start;
		iter->end_addr = new_end;
		return 1;
	}
}

template <class ADDRESS>
int CacheAlgo<ADDRESS>::modify_update(ADDRESS start_addr, ADDRESS end_addr, ADDRESS new_start, ADDRESS new_end) {
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = search_update_iter(start_addr, end_addr);
	if (iter == ranges.end() )
		return -1;
	else {
		iter->start_addr = new_start;
		iter->end_addr = new_end;
		return 1;
	}
}

template <class ADDRESS>
int CacheAlgo<ADDRESS>::insert(ADDRESS start_addr, ADDRESS end_addr) {
	if (check_overlap(start_addr, end_addr) )
		return -1;
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = ranges.begin();
	Range<ADDRESS> temp;
	temp.start_addr = start_addr;
	temp.end_addr = end_addr;
	ranges.insert(iter, temp);
	return 1;
}

template <class ADDRESS>
int CacheAlgo<ADDRESS>::remove(ADDRESS start_addr, ADDRESS end_addr) {
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = search_iter(start_addr, end_addr);
	if (iter == ranges.end() )
		return -1;
	else {
		ranges.erase(iter);
		return 1;
	}
}

template <class ADDRESS>
bool CacheAlgo<ADDRESS>::search(ADDRESS start_addr, ADDRESS end_addr) {
	if (search_iter(start_addr, end_addr) == ranges.end() )
		return false;
	else
		return true;
}

template<class ADDRESS>
bool CacheAlgo<ADDRESS>::search_update(ADDRESS start_addr, ADDRESS end_addr) {
	if (search_update_iter(start_addr, end_addr) == ranges.end() )
		return false;
	else
		return true;
}

template<class ADDRESS>
void CacheAlgo<ADDRESS>::clear () {
	ranges.clear();
}

template<class ADDRESS>
Range<ADDRESS> CacheAlgo<ADDRESS>::kickout() {
	Range<ADDRESS> temp;
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = ranges.end() - 1;
	temp.start_addr = iter->start_addr;
	temp.end_addr = iter->end_addr;
	ranges.erase(iter);
	return temp;
}

template<class ADDRESS>
void CacheAlgo<ADDRESS>::print() {
	typename deque< Range<ADDRESS> >::iterator iter;
	iter = ranges.begin();
	while (iter != ranges.end() ) {
		cout << "start_addr:\t" << iter->start_addr << endl;
		cout << "end_addr:\t" << iter->end_addr << endl << endl;
		iter++;
	}
}

template <class ADDRESS>
typename deque< Range<ADDRESS> >::iterator CacheAlgo<ADDRESS>::search_iter(ADDRESS start_addr, ADDRESS end_addr) {
	typename deque< Range<ADDRESS> >::iterator iter = ranges.begin();
	while (iter != ranges.end() && iter->start_addr != start_addr)
		iter++;
	if (iter == ranges.end() || iter->end_addr != end_addr) {
		return ranges.end();
	}
	return iter;
}

template <class ADDRESS>
typename deque< Range<ADDRESS> >::iterator CacheAlgo<ADDRESS>::search_update_iter(ADDRESS start_addr, ADDRESS end_addr) {	
	typename deque< Range<ADDRESS> >::iterator iter = ranges.begin();
	iter = search_iter(start_addr, end_addr);
	if (iter == ranges.end() )
		return iter;
	else {
		Range<ADDRESS> temp;
		temp.start_addr = iter->start_addr;
		temp.end_addr = iter->end_addr;
		ranges.erase(iter);
		iter = ranges.begin();
		iter = ranges.insert(iter, temp);
		return iter;
	}
}

template <class ADDRESS>
bool CacheAlgo<ADDRESS>::check_overlap (ADDRESS start_addr, ADDRESS end_addr) {
	typename deque< Range<ADDRESS> >::iterator iter = ranges.begin();
	iter = ranges.begin();
	while (iter != ranges.end() ) {
		if (iter->start_addr < start_addr && iter->end_addr >= start_addr)
			return true;
		if (iter->start_addr <= end_addr && iter->end_addr > end_addr)
			return true;
		iter++;
	}
	return false;
}
