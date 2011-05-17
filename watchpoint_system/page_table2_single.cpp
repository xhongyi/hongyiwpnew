#ifndef PAGE_TABLE2_SINGLE_CPP_
#define PAGE_TABLE2_SINGLE_CPP_

#include "page_table2_single.h"

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single(PageTable1_single<ADDRESS, FLAGS> &pt1_ref) {
   pt1 = &pt1_ref;
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0;
      superpage_unwatched[i] = 0xff;
   }
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single() {
   pt1 = NULL;
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0;
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
void PageTable2_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   if (target_flags) {
      ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
      ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
      for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
         if (check_unity(i, true)) {
            superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));
            superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
         }
         else {
            superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
         }
      }
      all_watched = true;
      all_unwatched = true;
      for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
         if (superpage_watched[i] != 0xff)
            all_watched = false;
         if (superpage_unwatched[i] != 0xff)
            all_unwatched = false;
      }
   }
}

template<class ADDRESS, class FLAGS>
void PageTable2_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   if (target_flags) {
      ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
      ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
      for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
         if (check_unity(i, false)) {
            superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
            superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));
         }
         else {
            superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
         }
      }
      all_watched = true;
      all_unwatched = true;
      for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
         if (superpage_watched[i] != 0xff)
            all_watched = false;
         if (superpage_unwatched[i] != 0xff)
            all_unwatched = false;
      }
   }
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
