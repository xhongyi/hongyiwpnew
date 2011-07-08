#ifndef PAGE_TABLE2_SINGLE_CPP_
#define PAGE_TABLE2_SINGLE_CPP_

#include "page_table2_single.h"

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single(Virtual_wp<ADDRESS, FLAGS> *pt1_ref) {
   pt1 = pt1_ref;
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
   /*
   all_watched = false;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      superpage_watched[i] = 0x00;
      superpage_unwatched[i] = 0xff;
   }
   */
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::~PageTable2_single() {
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   // checking the highest level hits
   if (all_watched)
      return ALL_WATCHED;
   else if (all_unwatched)
      return ALL_UNWATCHED;
   // checking superpage hits
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
   if (pt1->general_fault(start_addr, end_addr, target_flags))
      return PAGETABLE_WATCHED;
   return PAGETABLE_UNWATCHED;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr);
}

// version 2: count all related pages
template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   //   calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   if (superpage_number_end > superpage_number_start) {                                                              // if superpage: start != end
      // first superpage
      if (check_unity(superpage_number_start, true)) {                                                               // if set to watched
         if (   ( (superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH]   & (1<<(superpage_number_start&0x7))) == 0 )
             || ( (superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_start&0x7))) != 0 )   )
            changes++;
         superpage_watched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_start&0x7));      // watched = 1
         superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));   // unwatched = 0
      }
      else {                                                                                                         // if set to unknown
         if (superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_start&0x7)) )// if orig_unwatched
            changes += (1<<SECOND_LEVEL_PAGE_NUM_LENGTH);                                                            // set all 4K bits again
         else                                                                                                        // if orig_unknown
            changes += ((superpage_number_start+1) << SECOND_LEVEL_PAGE_NUM_LENGTH) - page_number_start;
         if ( (superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_start&0x7))) != 0 )
            changes++;
         superpage_unwatched[superpage_number_start>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_start&0x7));   // unwatched = 0
      }
      // middle superpages
      for (ADDRESS i=superpage_number_start+1;i<superpage_number_end;i++) {
         if (   ( (superpage_watched[i>>BIT_MAP_OFFSET_LENGTH]   & (1<<(i&0x7))) == 0 )
             || ( (superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) != 0 )   )
            changes++;
         superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));                                                // watched = 1
         superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                                             // unwatched = 0
      }
      // last superpage
      if (check_unity(superpage_number_end, true)) {                                                                 // if set to watched
         if (   ( (superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH]   & (1<<(superpage_number_end&0x7))) == 0 )
             || ( (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7))) != 0 )   )
            changes++;
         superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_end&0x7));          // watched = 1
         superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));       // unwatched = 0
      }
      else {                                                                                                         // if set to unknown
         if (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7)) )    // if orig_unwatched
            changes += (1<<SECOND_LEVEL_PAGE_NUM_LENGTH);                                                            // set all 4K bits again
         else                                                                                                        // if orig_unknown
            changes += page_number_end - (superpage_number_end << SECOND_LEVEL_PAGE_NUM_LENGTH) + 1;
         if ( (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7))) != 0 )
            changes++;
         superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));       // unwatched = 0
      }
   }
   else {                                                                                                            // if superpage start == end
      if (check_unity(superpage_number_start, true)) {                                                               // if set to watched
         if (   ( (superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH]   & (1<<(superpage_number_end&0x7))) == 0 )
             || ( (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7))) != 0 )   )
            changes++;
         superpage_watched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] |= (1<<(superpage_number_end&0x7));          // watched = 1
         superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));       // unwatched = 0
      }
      else {                                                                                                         // if set to unknown
         if (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7)) )    // if orig_unwatched
            changes += (1<<SECOND_LEVEL_PAGE_NUM_LENGTH);                                                            // set all 4K bits again
         else                                                                                                        // if orig_unknown
            changes += page_number_end - page_number_start + 1;
         if ( (superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] & (1<<(superpage_number_end&0x7))) != 0 )
            changes++;
         superpage_unwatched[superpage_number_end>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(superpage_number_end&0x7));       // unwatched = 0
      }
   }
   bool old_all_watched = all_watched;
   bool old_all_unwatched = all_unwatched;
   // all_watched/all_unwatched (check superpage unity)
   all_watched = true;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      if (superpage_watched[i] != 0xff)
         all_watched = false;
      if (superpage_unwatched[i] != 0xff)
         all_unwatched = false;
   }
   if (all_watched || all_unwatched) {
      if (old_all_watched != all_watched)
         changes = 0;
      else
         changes = 1;
      if (old_all_unwatched != all_unwatched)
         changes = 0;
      else
         changes = 1;
   }
   return changes;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH)+1;
   // superpages
   ADDRESS page_number = ((start_addr>>PAGE_OFFSET_LENGTH)<<PAGE_OFFSET_LENGTH);
   for (ADDRESS i=superpage_number_start;i!=superpage_number_end;i++) {
      if (check_unity(i, false)) {                                         // if set to unwatched  
         if (   ( (superpage_watched[i>>BIT_MAP_OFFSET_LENGTH]   & (1<<(i&0x7))) != 0 )
             || ( (superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) == 0 )   )
            changes++;
         superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));     // watched = 0
         superpage_unwatched[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));    // unwatched = 1
         page_number = ((i+1)<<SUPERPAGE_OFFSET_LENGTH);
      }
      else {
         while ((page_number != ((i+1)<<SUPERPAGE_OFFSET_LENGTH)) && (page_number <= end_addr)) { // for all pages within the range and this superpage
            if (!(pt1->watch_fault(page_number, page_number)) ) {
               changes++;
               superpage_watched[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));     // watched = 0
            }
            page_number += PAGE_SIZE;
         }
      }
   }
   bool old_all_watched = all_watched;
   bool old_all_unwatched = all_unwatched;
   // all_watched/all_unwatched
   all_watched = true;
   all_unwatched = true;
   for (int i=0;i<SUPER_PAGE_BIT_MAP_NUMBER;i++) {
      if (superpage_watched[i] != 0xff)
         all_watched = false;
      if (superpage_unwatched[i] != 0xff)
         all_unwatched = false;
   }
   if (all_watched || all_unwatched) {
      if (old_all_watched != all_watched)
         changes = 0;
      else
         changes = 1;
      if (old_all_unwatched != all_unwatched)
         changes = 0;
      else
         changes = 1;
   }
   return changes;
}

template<class ADDRESS, class FLAGS>
bool PageTable2_single<ADDRESS, FLAGS>::check_unity(ADDRESS superpage_number, bool watched) {
   ADDRESS start = (superpage_number<<SUPERPAGE_OFFSET_LENGTH);
   ADDRESS end = ((superpage_number+1)<<SUPERPAGE_OFFSET_LENGTH);
   for (ADDRESS i=start;i!=end;i+=PAGE_SIZE) {
      if (pt1->watch_fault(i, i) != watched)
         return false;
   }
   return true;
}

template<class ADDRESS, class FLAGS>
void PageTable2_single<ADDRESS, FLAGS>::watch_print(ostream &output) {
}

#endif
