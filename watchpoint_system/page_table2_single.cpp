#ifndef PAGE_TABLE2_SINGLE_CPP_
#define PAGE_TABLE2_SINGLE_CPP_

#include "page_table2_single.h"

template<class ADDRESS, class FLAGS>
PageTable2_single<ADDRESS, FLAGS>::PageTable2_single(Virtual_wp<ADDRESS, FLAGS> *pt1_ref) {
   pt1 = pt1_ref;
   all_watched = false;
   all_unwatched = true;
   superpage_watched.reset();
   superpage_unwatched.set();
   pagetable_watched.reset();
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
         if (superpage_watched[i])
            return SUPERPAGE_WATCHED;
         if (!superpage_unwatched[i])
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
   if (superpage_number_end > superpage_number_start) {        // if superpage: start != end
      // first superpage
      if (check_unity(superpage_number_start, true)) {         // if set to watched
         if (!superpage_watched[superpage_number_start] || superpage_unwatched[superpage_number_start]) {
            changes++;
            superpage_watched.set(superpage_number_start);     // watched = 1
            superpage_unwatched.reset(superpage_number_start); // unwatched = 0
         }
      }
      else {                                                   // if set to unknown
         if (superpage_unwatched[superpage_number_start]) {    // if orig_unwatched
            changes += set_unknown(superpage_number_start);    // Everything was unwatched, now SOME of the
                                                               // lower pages are changed to watched.
            superpage_unwatched.reset(superpage_number_start); // unwatched = 0
         }
         else                                                  // if orig_unknown
            changes += set_unknown(superpage_number_start);
      }
      // middle superpages
      for (ADDRESS i=superpage_number_start+1;i<superpage_number_end;i++) {
         if (!superpage_watched[i] || superpage_unwatched[i]) {
            changes++;
            superpage_watched.set(i);                          // watched = 1
            superpage_unwatched.reset(i);                      // unwatched = 0
         }
      }
      // last superpage
      if (check_unity(superpage_number_end, true)) {           // if set to watched
         if (!superpage_watched[superpage_number_end] || superpage_unwatched[superpage_number_end]) {
            changes++;
            superpage_watched.set(superpage_number_end);       // watched = 1
            superpage_unwatched.reset(superpage_number_end);   // unwatched = 0
         }
      }
      else {                                                   // if set to unknown
         if (superpage_unwatched[superpage_number_end]) {      // if orig_unwatched
            changes += set_unknown(superpage_number_end);      // Everything was unwatched, now SOME of the
                                                               // lower pages are changed to watched.
            superpage_unwatched.reset(superpage_number_end);   // unwatched = 0
         }
         else                                                  // if orig_unknown
            changes += set_unknown(superpage_number_end);
      }
   }
   else {                                                      // if superpage start == end
      if (check_unity(superpage_number_start, true)) {         // if set to watched
         if (!superpage_watched[superpage_number_end] || superpage_unwatched[superpage_number_end]) {
            changes++;
            superpage_watched.set(superpage_number_end);       // watched = 1
            superpage_unwatched.reset(superpage_number_end);   // unwatched = 0
         }
      }
      else {                                                   // if set to unknown
         if (superpage_unwatched[superpage_number_end]) {      // if orig_unwatched
            changes += set_unknown(superpage_number_end);      // Everything was unwatched, now SOME of the
                                                               // lower pages are changed to watched.
            superpage_unwatched.reset(superpage_number_end);   // unwatched = 0
         }
         else                                                  // if orig_unknown
            changes += set_unknown(superpage_number_end);
      }
   }
   // check all_watched/unwatched
   if (superpage_watched.count() == SUPERPAGE_NUM) {
      if (!all_watched) {
         // Technically, we should only set "1" here (top-most level).
         // However, let's think of the case where we just set everyone to watched
         // And next, below in rm_watchpoint, we remove one single page. In that case, we should
         // set all the mid-levels in rm_watchpoint. What we'll calculate here, instead, is
         // setting the middle levels here instead of there, to make the logic easier.
         changes += 1; // We must still set the top-most level.
         all_watched = true;
         all_unwatched = false;
      }
      else
         changes = 0;
   }
   else if (superpage_unwatched.count() == SUPERPAGE_NUM) {
      /*if (!all_unwatched) {
         changes = 1;
         all_watched = false;
         all_unwatched = true;
      }
      else
         changes = 0;
      Seriously, we should never get here. You can't set a watchpoint in a virtual memory system
      and end up with no watchpoints. */
      fprintf(stderr, "Somehow, SETTING watchpoints has caused us to say that nothing is watched..\n");
      assert(0);
   }
   else {
      if (all_watched || all_unwatched) {
         if (all_watched) // all_unwatched doesn't mean much. However, all_watched means that CR3 was set unavail. It's not avail, so add one more change.
            changes += 1;
         all_watched = false;
         all_unwatched = false;
      }
   }
   // refresh internal pagetable history (does not count)
   ADDRESS pagetable_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS pagetable_end = (end_addr>>PAGE_OFFSET_LENGTH);
   for (ADDRESS i=pagetable_start;i<=pagetable_end;i++)
      pagetable_watched.set(i);
   return changes;
}

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH)+1;
   for (ADDRESS i=superpage_number_start;i!=superpage_number_end;i++) {
      if (check_unity(i, false)) {                             // if set to unwatched  
         if (superpage_watched[i] || !superpage_unwatched[i]) {
            changes++;
            superpage_watched.reset(i);                        // watched = 0
            superpage_unwatched.set(i);                        // unwatched = 1
         }
         changes += set_unknown(i);                            // Anything at the lower level needs to be cleared out so that our "unwatched" status goes through to the PTE.
      }
      else {                                                   // if set to unknown
         if (superpage_watched[i]) {                           // if originally watched
             int temp_changes = set_unknown(i);                // Fixup for any pages that have changed.
             if(temp_changes) {
                 // If any over the lower-level pages changed, we need to count their bit removal
                 changes+=temp_changes;
                 // We also need to count setting this super-page to not-watched.
                 changes++;
                 superpage_watched.reset(i);                   // watched = 0
             }
         }
         else                                                  // if originally unknown
            changes += set_unknown(i);
      }
   }
   // check all_watched/unwatched
   if (superpage_watched.count() == SUPERPAGE_NUM) {
      if (!all_watched) {
         changes = 1;
         all_watched = true;
         all_unwatched = false;
      }
      else
         changes = 0;
   }
   else if (superpage_unwatched.count() == SUPERPAGE_NUM) {
      if (!all_unwatched) {
         // changes = 1; // don't need to do this. Setting "everything unwatched" doesn't change any bits at the top level.
         all_watched = false;
         all_unwatched = true;
      }
      else
         changes = 0;
   }
   else {
      if (all_watched && changes > 0) {
         if (all_watched)
            changes += 1;
         all_watched = false;
         all_unwatched = false;
      }
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

template<class ADDRESS, class FLAGS>
int PageTable2_single<ADDRESS, FLAGS>::set_unknown(ADDRESS superpage_number) {
   int changes = 0;
   for (ADDRESS i=(superpage_number<<SUPERPAGE_OFFSET_LENGTH);
                i!=((superpage_number+1)<<SUPERPAGE_OFFSET_LENGTH);
                i+=PAGE_SIZE) {                                // for all pages contains in this superpage
      if (pt1->watch_fault(i, i)) {                            // if this page is watched
         if (!pagetable_watched[i>>PAGE_OFFSET_LENGTH]) {
            changes++;
            pagetable_watched.set(i>>PAGE_OFFSET_LENGTH);      // watched = 1
         }
      }
      else {
         if (pagetable_watched[i>>PAGE_OFFSET_LENGTH]) {
            changes++;
            pagetable_watched.reset(i>>PAGE_OFFSET_LENGTH);
         }
      }
   }
   return changes;
}

#endif
