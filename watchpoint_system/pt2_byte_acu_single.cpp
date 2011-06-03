#ifndef PT2_BYTE_ACU_SINGLE_CPP_
#define PT2_BYTE_ACU_SINGLE_CPP_

#include "pt2_byte_acu_single.h"

template<class ADDRESS, class FLAGS>
PT2_byte_acu_single<ADDRESS, FLAGS>::PT2_byte_acu_single(Oracle<ADDRESS, FLAGS> *wp_ref) {
	wp = wp_ref;
   seg_reg_watched = false;
   seg_reg_unwatched = true;
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
   if (seg_reg_watched)
      return ALL_WATCHED;
   else if (seg_reg_unwatched)
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
	   return PAGETABLE_UNWATCHED;
	if (wp->watch_fault(start_addr, end_addr))
	   return BITMAP_WATCHED;
	return BITMAP_UNWATCHED;
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
int PT2_byte_acu_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   unsigned int firstpage_bitmap_change = 0, 
                lastpage_bitmap_change = 0, 
                first_superpage_bitmap_change = 0, 
                last_superpage_bitmap_change = 0;
   pt_unwatched[page_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number_start&0x7));                          // unwatched = 0
   pt_unwatched[page_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number_end&0x7));                              // unwatched = 0
   superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));         // unwatched = 0
   superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));             // unwatched = 0
   seg_reg_unwatched = false;
   if (page_number_start == page_number_end) {  // if in the same page
      if (check_page_level_unity(page_number_start, true)) {
         pt_watched[page_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number_start&0x7));                          //	watched = 1
         if (check_superpage_level_unity(superpage_number_start, true)) {
            superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
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
         firstpage_bitmap_change = 1;
      }
      else
         firstpage_bitmap_change = ((page_number_start+1)<<PAGE_OFFSET_LENGTH) - start_addr;
      if (check_page_level_unity(page_number_end, true)) {
         pt_watched[page_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number_end&0x7));                              //	watched = 1
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
         if (check_superpage_level_unity(superpage_number_start, true)) {
            superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
            if (check_seg_reg_level_unity(true))
               seg_reg_watched = true;
            return 1;
         }
         return page_number_end - page_number_start - 1 + firstpage_bitmap_change + lastpage_bitmap_change;
      }
      else {   // if not in the same superpage
         // set start and end superpage
         if (check_superpage_level_unity(superpage_number_start, true)) {
            superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
            first_superpage_bitmap_change = 1;
         }
         else
            first_superpage_bitmap_change = ((superpage_number_start+1)<<SECOND_LEVEL_PAGE_NUM_LENGTH) - page_number_start;
         if (check_superpage_level_unity(superpage_number_end, true)) {
            superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_end&0x7));          // watched = 1
            last_superpage_bitmap_change = 1;
         }
         else
            last_superpage_bitmap_change = page_number_end - (superpage_number_end<<SECOND_LEVEL_PAGE_NUM_LENGTH) + 1;
         // set all superpages in the middle
         for (ADDRESS i=superpage_number_start+1;i!=superpage_number_end;i++) {
            superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));                                                // watched = 1
            superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                                             // unwatched = 0
         }
         if (check_seg_reg_level_unity(true)) {
            seg_reg_watched = true;
            return 1;
         }
         return firstpage_bitmap_change + lastpage_bitmap_change 
              + first_superpage_bitmap_change + last_superpage_bitmap_change 
              + superpage_number_end - superpage_number_start - 1;
      }
   }
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   unsigned int page_change = 0, superpage_change = 0, total_change = 0;
   ADDRESS page_number = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS superpage_number = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   for (ADDRESS i=start_addr;i<=end_addr;i++) {
      if (!wp->watch_fault(i, i))      // if unwatched
         page_change++;
      if ( (i & (PAGE_SIZE-1)) == (PAGE_SIZE-1) ) {   // if page_end
         if ((page_change == PAGE_SIZE) || check_page_level_unity(page_number, false)) {     // if all unwatched in a page
            pt_unwatched[page_number>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number&0x7));      //	unwatched = 1
            pt_watched[page_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number&0x7));       // watched = 0
            superpage_change++;
         }
         else if (page_change) {
            pt_watched[page_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number&0x7));       // watched = 0
            superpage_change++;
            superpage_change += page_change;
         }
         page_change = 0;
         page_number++;
      }
      if ( (i & ((1<<SUPERPAGE_OFFSET_LENGTH)-1)) == ((1<<SUPERPAGE_OFFSET_LENGTH)-1) ) {    // if superpage_end
         if (check_superpage_level_unity(superpage_number, false)) {
            superpage_unwatched[superpage_number>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number&0x7));    // unwatched = 1
            superpage_watched[superpage_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number&0x7));     // watched = 0
            total_change++;
         }
         else if (superpage_change) {
            superpage_watched[superpage_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number&0x7));     // watched = 0
            total_change++;
            total_change += superpage_change;
         }
         superpage_change = 0;
         superpage_number++;
      }
   }
   if (page_change) {
      pt_watched[page_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(page_number&0x7));          // watched = 0
      if (check_page_level_unity(page_number, false)) {
         pt_unwatched[page_number>>BIT_MAP_OFFSET_LENGTH] |= (1<<(page_number&0x7));      //	unwatched = 1
         superpage_change++;
      }
      else {
         superpage_change += page_change;
      }
   }
   if (superpage_change) {
      superpage_watched[superpage_number>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number&0x7));     // watched = 0
      if (check_superpage_level_unity(superpage_number, false)) {
         superpage_unwatched[superpage_number>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number&0x7));    // unwatched = 1
         total_change++;
      }
      else {
         total_change += superpage_change;
       }
   }
   if (total_change) {
      seg_reg_watched = false;
      if (check_seg_reg_level_unity(false)) {
         seg_reg_unwatched = true;
         return 1;
      }
   }
   return total_change;
}

template<class ADDRESS, class FLAGS>
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_page_level_unity(ADDRESS page_number, bool watched) {
   ADDRESS start = (page_number<<PAGE_OFFSET_LENGTH);
   ADDRESS end   = ((page_number+1)<<PAGE_OFFSET_LENGTH);
   for (ADDRESS i=start;i!=end;i++) {
      if (wp->watch_fault(i, i) != watched)
         return false;
   }
   return true;
}

template<class ADDRESS, class FLAGS>
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_superpage_level_unity(ADDRESS superpage_number, bool watched) {
   ADDRESS page_number_start = (superpage_number<<SECOND_LEVEL_PAGE_NUM_LENGTH);
   ADDRESS page_number_end   = ((superpage_number+1)<<SECOND_LEVEL_PAGE_NUM_LENGTH);
   if (watched) {
      for (ADDRESS i=page_number_start;i!=page_number_end;i++) {
         if (!(pt_watched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))
            return false;
      }
   }
   else {
      for (ADDRESS i=page_number_start;i!=page_number_end;i++) {
         if (!(pt_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))
            return false;
      }
   }
   return true;
}

template<class ADDRESS, class FLAGS>
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_seg_reg_level_unity(bool watched) {
   if (watched) {
      for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
         if (superpage_watched[i] != 0xff)
            return false;
      }
   }
   else {
      for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
         if (superpage_unwatched[i] != 0xff)
            return false;
      }
   }
   return true;
}

#endif
