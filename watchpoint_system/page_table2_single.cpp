#ifndef PAGE_TABLE2_SINGLE_CPP_
#define PAGE_TABLE2_SINGLE_CPP_

#include "page_table2_single.h"

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single(Virtual_wp<ADDRESS, FLAGS> *pt1_ref) {
   pt1 = pt1_ref;
   all_watched = false;
   all_readonly = false;
   superpage_watched.reset();
   superpage_readonly.reset();
   pagetable_watched.reset();;
   pagetable_readonly.reset();
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single() {
   pt1 = NULL;
}

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::~PageTable2_single() {
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   bool check_readonly = (target_flags & WA_WRITE);
   // checking the highest level hits
   if (all_watched)
      return ALL_WATCHED;
   else if (check_readonly && all_readonly)
      return ALL_READONLY;
   // checking superpage hits
   else {
      ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
      ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
      for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
         if (superpage_watched[i])
            return SUPERPAGE_WATCHED;
         if (check_readonly && superpage_readonly[i])
            return SUPERPAGE_READONLY;
      }
   }
   ADDRESS page_num_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_num_end   = (end_addr  >>PAGE_OFFSET_LENGTH);
   for (ADDRESS i=page_num_start;i<=page_num_end;i++) {
      if (pagetable_watched[i])
         return PAGETABLE_WATCHED;
      if (check_readonly && pagetable_readonly[i])
         return PAGETABLE_READONLY;
   }
   return AVAILABLE;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, WA_READ|WA_WRITE);
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, WA_READ);
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, WA_WRITE);
}

// version 2: count all related pages
template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   if (superpage_number_end > superpage_number_start) {        // if superpage: start != end
      // first superpage
      if (check_unity(superpage_number_start, true)) {         // if set to watched
         if (!superpage_watched[superpage_number_start]) {
            changes++;
            superpage_watched.set(superpage_number_start);     // watched = 1
         }
      }
      else if (check_unity(superpage_number_start, false)) {
         if (!superpage_readonly[superpage_number_start]) { // if orig not r/o
            changes++;
            superpage_readonly.set(superpage_number_start); // unwatched = 0
         }
      }
      else {
         assert(!superpage_watched[superpage_number_start] && !superpage_readonly[superpage_number_start]);
         changes += set_available(superpage_number_start);
      }
      // middle superpages
      for (ADDRESS i=superpage_number_start+1;i<superpage_number_end;i++) {
         if (target_flags & WA_READ) {
            if (!superpage_watched[i]) {
               changes++;
               superpage_watched.set(i);                          // watched = 1
            }
         }
         else if (target_flags & WA_WRITE) {
            if (!superpage_readonly[i]) {
               changes++;
               superpage_readonly.set(i);
            }
         }
      }
      // last superpage
      if (check_unity(superpage_number_end, true)) {         // if set to watched
         if (!superpage_watched[superpage_number_end]) {
            changes++;
            superpage_watched.set(superpage_number_end);     // watched = 1
         }
      }
      else if (check_unity(superpage_number_end, false)) {
         if (!superpage_readonly[superpage_number_end]) { // if orig not r/o
            changes++;
            superpage_readonly.set(superpage_number_end); // unwatched = 0
         }
      }
      else {
         assert(!superpage_watched[superpage_number_end] && !superpage_readonly[superpage_number_end]);
         changes += set_available(superpage_number_end);
      }
   }
   else {                                                      // if superpage start == end
      if (check_unity(superpage_number_start, true)) {         // if set to watched
         if (!superpage_watched[superpage_number_start]) {
            changes++;
            superpage_watched.set(superpage_number_start);     // watched = 1
         }
      }
      else if (check_unity(superpage_number_start, false)) {
         if (!superpage_readonly[superpage_number_start]) { // if orig not r/o
            changes++;
            superpage_readonly.set(superpage_number_start); // unwatched = 0
         }
      }
      else {
         assert(!superpage_watched[superpage_number_start] && !superpage_readonly[superpage_number_start]);
         changes += set_available(superpage_number_start);
      }
   }
   // check CR3 register changes
   if (check_unity(true)) {
      if (!all_watched) {
         changes = 1;
         all_watched = true;
      }
      else
         changes = 0;
   }
   else if (check_unity(false)) {
      if (!all_readonly) {
         changes = 1;
         all_readonly = true;
      }
      else
         changes = 0;
   }
   return changes;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH)+1;
   for (ADDRESS i=superpage_number_start;i!=superpage_number_end;i++) {
      if (check_unity(i, true)) {         // if set to watched
         if (!superpage_watched[i]) {
            changes++;
            superpage_watched.set(i);     // watched = 1
         }
      }
      else if (check_unity(i, false)) {   // if set to r/o
         if (!superpage_readonly[i]) {
            changes++;
            superpage_readonly.set(i);    // readonly = 1
            superpage_watched.reset(i);
         }
      }
      else {
         if (superpage_watched[i] || superpage_readonly[i]) {
            changes++;
            superpage_watched.reset(i);
            superpage_readonly.reset(i);
         }
         changes += set_available(i);
      }
   }
   // check CR3 register changes
   if (check_unity(true)) {
      if (!all_watched) {
         changes = 1;
         all_watched = true;
      }
      else
         changes = 0;
   }
   else if (check_unity(false)) {
      if (!all_readonly) {
         changes = 1;
         all_readonly = true;
         all_watched = false;
      }
      else
         changes = 0;
   }
   else if (all_watched || all_readonly) {
      changes++;
      all_watched = false;
      all_readonly = false;
   }
   return changes;
}

template<class ADDRESS, class FLAGS>
bool PageTable2_single<ADDRESS, FLAGS>::check_unity(ADDRESS superpage_number, bool watched) {
   ADDRESS start = (superpage_number<<SUPERPAGE_OFFSET_LENGTH);
   ADDRESS end = ((superpage_number+1)<<SUPERPAGE_OFFSET_LENGTH);
   for (ADDRESS i=start;i!=end;i+=PAGE_SIZE) {
      if (!(pt1->write_fault(i, i)))            // pagetable is not marked as r/o or watched
         return false;
      if (watched && !(pt1->read_fault(i, i)))  // pagetable is not marked as watched
         return false;
   }
   return true;
}

template<class ADDRESS, class FLAGS>
bool PageTable2_single<ADDRESS, FLAGS>::check_unity(bool watched) {
   for (ADDRESS i=0;i<SUPERPAGE_NUM;i++) {
      if (!superpage_watched[i])
         return false;
      if (watched && !superpage_readonly[i])
         return false;
   }
   return true;
}

template<class ADDRESS, class FLAGS>
void PageTable2_single<ADDRESS, FLAGS>::watch_print(ostream &output) {
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::set_available(ADDRESS superpage_number) {
   int changes = 0;
   for (ADDRESS i=(superpage_number<<SUPERPAGE_OFFSET_LENGTH);
                i!=((superpage_number+1)<<SUPERPAGE_OFFSET_LENGTH);
                i+=PAGE_SIZE) {                                // for all pages contains in this superpage
      if (pt1->watch_fault(i, i)) {                            // if this page is watched or r/o
         if (pt1->read_fault(i, i)) {                          // if this page is watched
            if (!pagetable_watched[i>>PAGE_OFFSET_LENGTH]) {
               changes++;
               pagetable_watched.set(i>>PAGE_OFFSET_LENGTH);   // watched = 1
            }
         }
         else {
            if (!pagetable_readonly[i>>PAGE_OFFSET_LENGTH]) {  // if this page is r/o
               changes++;
               pagetable_readonly.set(1>>PAGE_OFFSET_LENGTH);  // r/o = 1
            }
         }
      }
      else {                                                   // if available
         if (pagetable_watched[i>>PAGE_OFFSET_LENGTH] || pagetable_readonly[i>>PAGE_OFFSET_LENGTH]) {
            changes++;
            pagetable_watched.reset(i>>PAGE_OFFSET_LENGTH);
            pagetable_readonly.reset(i>>PAGE_OFFSET_LENGTH);
         }
      }
   }
   return changes;
}

#endif
