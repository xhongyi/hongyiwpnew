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
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   unsigned int change = 0;
   unsigned int firstpage_bitmap_change = 0, 
                lastpage_bitmap_change = 0, 
                first_superpage_bitmap_change = 0, 
                last_superpage_bitmap_change = 0;
   if (page_number_start == page_number_end) {  // if in the same page
      pt_unwatched[page_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number_start&0x7));                          // unwatched = 0
      if (check_page_level_unity(page_number_start, true)) {
         pt_watched[page_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number_start&0x7));                          //	watched = 1
         superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));      // unwatched = 0
         if (check_superpage_level_unity(superpage_number_start, true)) {
            superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
            seg_reg_unwatched = false;
            if (check_seg_reg_level_unity(true))
               seg_reg_watched = true;
         }
         return 1;
      }
      else
         return end_addr - start_addr + 1;
   }
   else {   // if not in the same page
      // setting start and end pagetables
      if (check_page_level_unity(page_number_start, true)) {
         pt_watched[page_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number_start&0x7));                          //	watched = 1
         pt_unwatched[page_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number_start&0x7));                       // unwatched = 0
         firstpage_bitmap_change = 1;
      }
      else
         firstpage_bitmap_change = ((page_number_start+1)<<PAGE_OFFSET_LENGTH) - start_addr;
      if (check_page_level_unity(page_number_end, true)) {
         pt_watched[page_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number_end&0x7));                              //	watched = 1
         pt_unwatched[page_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number_end&0x7));                           // unwatched = 0
         lastpage_bitmap_change = 1;
      }
      else
         lastpage_bitmap_change = end_addr - (page_number_end<<PAGE_OFFSET_LENGTH) + 1;
      // setting all pagetables in the middle
      for (ADDRESS i=page_number_start+1;i!=page_number_end;i++) {
         pt_watched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));                              //	watched = 1
         pt_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                           // unwatched = 0
      }
      if (superpage_number_start == superpage_number_end) { // if in the same superpage
         superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));      // unwatched = 0
         if (check_superpage_level_unity(superpage_number_start, true)) {
            superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
            seg_reg_unwatched = false;
            if (check_seg_reg_level_unity(true))
               seg_reg_watched = true;
            return 1;
         }
         return page_number_end - page_number_start - 1 + firstpage_bitmap_change + lastpage_bitmap_change;
      }
      else {   // if not in the same superpage
         // set start and end superpage
         
         // set all superpages in the middle
      }
   }
}

#endif
