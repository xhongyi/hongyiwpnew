#ifdef PT2_BYTE_ACU_SINGLE_CPP_
#define PT2_BYTE_ACU_SINGLE_CPP_

#include "pt2_byte_acu_single.h"

template<class ADDRESS, class FLAGS>
PT2_byte_acu_single<ADDRESS, FLAGS>::PT2_byte_acu_single(Oracle<ADDRESS, FLAGS> *wp_ref) {
	wp = wp_ref;
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0x00;
      superpage_unwatched[i] = 0xff;
   }
	for (int i=0;i<BIT_MAP_NUMBER;i++) {
	   pt_watched[i] = 0;
	   pt_unwatched[i] = 0xff;
	}
}

template<class ADDRESS, class FLAGS>
PT2_byte_acu_single<ADDRESS, FLAGS>::PT2_byte_acu_single() {
	wp = NULL;
}

template<class ADDRESS, class FLAGS>
PT2_byte_acu_single<ADDRESS, FLAGS>::~PT2_byte_acu_single() {
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   bool unwatched = true;
   // checking the highest level hits
   if (all_watched)
      return ALL_WATCHED;
   else if (all_unwatched)
      return ALL_UNWATCHED;
   // checking superpage hits
   else {
      ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
      ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
      for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
         if (superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )
            return SUPERPAGE_WATCHED;
         if (!(superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))
            unwatched = false;
      }  // corner case here (mid = 00)
      if (unwatched)
         return SUPERPAGE_UNWATCHED;
   }
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   unwatched = true;
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {           //	for each page, 
		if (pt_watched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )         //	if one of them is watched, 
			return PAGETABLE_WATCHED;                                      //	then return watched.
		if (!(pt_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))    //	if one of them is not unwatched, 
			unwatched = false;                                             //	then switch unwatched to false
	}
	if (unwatched)
	   return PAGETABLE_WATCHED;
	return TRIE;
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags = 0) {
   
}

#endif
