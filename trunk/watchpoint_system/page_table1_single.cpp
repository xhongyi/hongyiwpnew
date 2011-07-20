#ifndef PAGE_TABLE1_SINGLE_CPP_
#define PAGE_TABLE1_SINGLE_CPP_

#include "page_table1_single.h"
#include <ostream>
#include <fstream>

using namespace std;

/*
 * Constructor
 * Initialize the wp pointer 
 * (wp should always be constructed and modified before the page table version)
 * And also initialize all pages to unwatched.
 */
template<class ADDRESS, class FLAGS>
PageTable1_single<ADDRESS, FLAGS>::PageTable1_single(Oracle<ADDRESS, FLAGS> *wp_ref) {
   wp = wp_ref;
   for (int i=0;i<BIT_MAP_NUMBER;i++) {
      bit_map[i] = 0;
      read_only_bitmap[i] = 0;
   }
}

template<class ADDRESS, class FLAGS>
PageTable1_single<ADDRESS, FLAGS>::PageTable1_single() {
   wp = NULL;
   for (int i=0;i<BIT_MAP_NUMBER;i++) {
      bit_map[i] = 0;
      read_only_bitmap[i] = 0;
   }
}

// Desctructor
template<class ADDRESS, class FLAGS>
PageTable1_single<ADDRESS, FLAGS>::~PageTable1_single() {
}

// print only pages being watched, used for debugging
template<class ADDRESS, class FLAGS>
void PageTable1_single<ADDRESS, FLAGS>::watch_print(ostream &output) {
   output <<"Start printing watchpoints..."<<endl;
   for (ADDRESS i=0;i<PAGE_NUMBER;i++) {
      if ( (bit_map[i>>3] & (1<<(i&0x7))) || (read_only_bitmap[i>>3] & (1<<(i&0x7))) ) {
         output <<"page number "<<i<<" is ";
         if((bit_map[i>>3] & (1<<(i&0x7))))
            output << "unavailable. It is ";
         else
            output << "available. It is ";

         if((read_only_bitmap[i>>3] & (1<<(i&0x7))))
            output << "read only." << endl;
         else
            output << "read-write." << endl;
      }
   }
   return;
}

template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::general_fault(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags, bool check_write) {
   // calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH)+1;
   for (ADDRESS i=page_number_start;i!=page_number_end;i++) {  // for each page, 
      if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)))    // if it is watched, 
         return true;                                          // then return true.
      else if (check_write && (read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) )
         return true;
   }
   return false;                                               // else return false.
}

template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, true);
}

template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::read_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, false);
}

template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::write_fault(ADDRESS start_addr, ADDRESS end_addr) {
   return general_fault(start_addr, end_addr, true);
}

// version 2: count all related pages  (faster)  (do not add them up---change WatchPoint.general_change() )
// add_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   // calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH)+1;
   for (ADDRESS i=page_number_start;i!=page_number_end;i++) {     // for each page, 
      bool made_a_change = false;
      if (target_flags & WA_WRITE) {
         if ( (read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) == 0 )
            made_a_change = true;
         read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));  // set the page read-only.
      }
      if (target_flags & WA_READ) {
         if ( (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) == 0 )
            made_a_change = true;
         bit_map[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));          // set the page watched. (overwrite)
      }

      if (made_a_change)
         changes++;
   }
   return changes;
}

// rm_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int changes = 0;
   // calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH)+1;
   for (ADDRESS i=page_number_start;i!=page_number_end;i++) {                             // for each page, 
      bool made_a_change = false;
      if (target_flags & WA_READ) {                                                       // If we're removing a read WP, try to set read-only.
         if (!wp->read_fault(i<<PAGE_OFFSET_LENGTH, ((i+1)<<PAGE_OFFSET_LENGTH)-1 ) ) {   // if it should not throw a fault
            if ( (read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) != 0 )
               made_a_change = true;
            read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                  // set the page unwatched (overwrite)
         }
      }
      if (target_flags & WA_WRITE) {                                                      // If we're removing a write WP, try to set completely avail.
         if (!wp->watch_fault(i<<PAGE_OFFSET_LENGTH, ((i+1)<<PAGE_OFFSET_LENGTH)-1 ) ) {
            if ( ((read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) != 0) ||
                  ((bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) != 0))
               made_a_change = true;
            read_only_bitmap[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                  // set the page unwatched (overwrite)
            bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));
         }
      }

      if (made_a_change)
         changes++;
   }
   return changes;
}

#endif
