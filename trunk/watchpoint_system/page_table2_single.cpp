#ifndef PAGE_TABLE2_SINGLE_CPP_
#define PAGE_TABLE2_SINGLE_CPP_

#include "page_table2_single.h"

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single(PageTable1_single<ADDRESS, FLAGS> &pt1_ref) {
   pt1 = &pt1_ref;
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0x00;
      superpage_unwatched[i] = 0xff;
   }
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single() {
   pt1 = NULL;
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0x00;
      superpage_unwatched[i] = 0xff;
   }
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::~PageTable2_single() {
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   if (all_watched)
      return ALL_WATCHED;
   else if (all_unwatched)
      return ALL_UNWATCHED;
   else {
      ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
      ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
      bool unwatched = true;
      for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
         if (superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )
            return SUPERPAGE_WATCHED;
         if (!(superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) ))
            unwatched = false;
      }  // corner case here (mid = 00)
      if (unwatched)
         return SUPERPAGE_UNWATCHED;
   }
   return MISSED;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   // first superpage
   if (check_unity(superpage_number_start, true)) {                                          // if set to watched
      changes++;
      superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
      superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));   // unwatched = 0
   }
   else {                                                                                    // if set to unknown
      if (superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_start&0x7)) )// if orig_unwatched
         changes += (1<<SECOND_LEVEL_PAGE_NUM_LENGTH);                                       // set all 1K bits again
      else if (superpage_number_end > superpage_number_start)                                // if orig_unknown
         changes += ((superpage_number_start+1) << SUPERPAGE_OFFSET_LENGTH) - start_addr;
      changes++;
      superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));   // unwatched = 0
   }
   // middle superpages
   for (ADDRESS i=superpage_number_start+1;i<superpage_number_end;i++) {
      changes++;
      superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));      // watched = 1
      superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));   // unwatched = 0
   }
   // last superpage
   if (superpage_number_end > superpage_number_start) {
      if (check_unity(superpage_number_end, true)) {                                            // if set to watched
         changes++;
         superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_end&0x7));        // watched = 1
         superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));     // unwatched = 0
      }
      else {                                                                                    // if set to unknown
         if (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7)) )  // if orig_unwatched
            changes += (1<<SECOND_LEVEL_PAGE_NUM_LENGTH);                                       // set all 1K bits again
         else                                                                                   // if orig_unknown
            changes += ((superpage_number_start+1) << SUPERPAGE_OFFSET_LENGTH) - start_addr;
         changes++;
         superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));   // unwatched = 0
      }
   }
   else
      changes += end_addr - start_addr + 1;
   // all_watched/all_unwatched
   all_watched = true;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      if (superpage_watched[i] != 0xff)
         all_watched = false;
      if (superpage_unwatched[i] != 0xff)
         all_unwatched = false;
   }
   if (all_watched || all_unwatched)
      changes = 1;
   return changes;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   // superpages
   ADDRESS mid_addr = start_addr;
   for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
      if (check_unity(superpage_number_start, false)) {                                         // if set to unwatched
         changes++;
         superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));     // watched = 0
         superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));    // unwatched = 1
      }
      else {
         while ((mid_addr < (i>>BIT_MAP_OFFSET_LENGTH)) && (mid_addr <= end_addr)) {
            if (!(pt1->watch_fault(mid_addr, mid_addr)) )
               changes++;
            mid_addr += (1<<PAGE_OFFSET_LENGTH);
         }
         changes++;
         superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));     // watched = 0
      }
   }
   // all_watched/all_unwatched
   all_watched = true;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      if (superpage_watched[i] != 0xff)
         all_watched = false;
      if (superpage_unwatched[i] != 0xff)
         all_unwatched = false;
   }
   if (all_watched || all_unwatched)
      changes = 1;
   return changes;
}

template<class ADDRESS, class FLAGS>
bool PageTable2_single<ADDRESS, FLAGS>::check_unity(ADDRESS superpage_number, bool watched) {
   ADDRESS start = (superpage_number<<SUPERPAGE_OFFSET_LENGTH);
   ADDRESS end = ((superpage_number+1)<<SUPERPAGE_OFFSET_LENGTH);
   for (ADDRESS i=start;i<end;i+=PAGE_SIZE) {
      if (pt1->watch_fault(i, i) != watched)
         return false;
   }
   return true;
}

#endif
