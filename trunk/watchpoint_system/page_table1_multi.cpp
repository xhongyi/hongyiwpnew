#ifndef PAGE_TABLE1_MULTI_CPP_
#define PAGE_TABLE1_MULTI_CPP_

#include "page_table1_multi.h"

template<class ADDRESS, class FLAGS>
PageTable1_multi<ADDRESS, FLAGS>::PageTable1_multi() {
   for (int i=0;i<BIT_MAP_NUMBER;i++)
      bit_map[i] = 0;
}

template<class ADDRESS, class FLAGS>
PageTable1_multi<ADDRESS, FLAGS>::~PageTable1_multi() {
}

template<class ADDRESS, class FLAGS>
void PageTable1_multi<ADDRESS, FLAGS>::start_thread(int32_t thread_id, PageTable1_single<ADDRESS, FLAGS>* ptr_in) {
   page_table_wp[thread_id] = ptr_in;
}

template<class ADDRESS, class FLAGS>
void PageTable1_multi<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   page_table_wp.erase(thread_id);
   for (ADDRESS i=0;i<PAGE_NUMBER;i++) {
      bool multi_page_table_fault = false;
      for (page_table_wp_iter=page_table_wp.begin();page_table_wp_iter!=page_table_wp.end();page_table_wp_iter++) {
         if (page_table_wp_iter->second->watch_fault(i<<PAGE_OFFSET_LENGTH, i<<PAGE_OFFSET_LENGTH) ) {
            multi_page_table_fault = true;
            break;
         }
      }
      if (!multi_page_table_fault)
		   bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
   }
}

template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {  //	for each page, 
		if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )   //	if it is watched, 
			return true;                                          //	then return true.
	}
	return false;                                               //	else return false.
}

template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

// version 2: count all related pages  (faster)  (do not add them up---change WatchPoint.general_change() )
//	add_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++)       //	for each page, 
		bit_map[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));          //	set the page watched. (overwrite)
	return page_number_end - page_number_start + 1;
}

//	rm_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_multi<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   //	calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   for (ADDRESS i=page_number_start;i<=page_number_end;i++) {                          //	for each page, 
      bool multi_page_table_fault = false;
      for (page_table_wp_iter=page_table_wp.begin();page_table_wp_iter!=page_table_wp.end();page_table_wp_iter++) {
         if (page_table_wp_iter->second->watch_fault(i<<PAGE_OFFSET_LENGTH, i<<PAGE_OFFSET_LENGTH) ) {
            multi_page_table_fault = true;
            break;
         }
      }
      if (!multi_page_table_fault) {
		   bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                           //	set the page unwatched (overwrite)
		   changes++;
      }
   }
   return changes;
}




#endif
