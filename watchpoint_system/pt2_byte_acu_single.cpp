#ifndef PT2_BYTE_ACU_SINGLE_CPP_
#define PT2_BYTE_ACU_SINGLE_CPP_

#include "pt2_byte_acu_single.h"

template<class ADDRESS, class FLAGS>
PT2_byte_acu_single<ADDRESS, FLAGS>::PT2_byte_acu_single(Virtual_wp<ADDRESS, FLAGS> *wp_ref) {
   wp = wp_ref;
   plb_misses = 0;
   seg_reg_read_watched = false;
   seg_reg_write_watched = false;
   seg_reg_unknown = false;
   superpage_read_watched.reset();
   superpage_write_watched.reset();
   superpage_unknown.reset();
   pt_read_watched.reset();
   pt_write_watched.reset();
   pt_unknown.reset();
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
   // update plb
   PLB_entry<ADDRESS> temp;
   temp.tag = 0;
   temp.level = 'a';
   if (plb.check_and_update(temp))
      plb_misses++;
   // checking the highest level hits
   if ((target_flags & WA_READ) && seg_reg_read_watched)
      return ALL_WATCHED;
   if ((target_flags & WA_WRITE) && seg_reg_write_watched)
      return ALL_WATCHED;
   if (!seg_reg_unknown)
      return ALL_UNWATCHED;
   // checking superpage hits
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end   = (end_addr  >>SUPERPAGE_OFFSET_LENGTH);
   for (ADDRESS i=superpage_number_start;i<=superpage_number_end;i++) {
      // update plb
      PLB_entry<ADDRESS> temp;
      temp.tag = i;
      temp.level = 's';
      if (plb.check_and_update(temp))
         plb_misses++;
      // check watched or not
      if ((target_flags & WA_READ) && superpage_read_watched[i])
         return SUPERPAGE_WATCHED;
      if ((target_flags & WA_WRITE) && superpage_write_watched[i])
         return SUPERPAGE_WATCHED;
      if (superpage_unknown[i])
         unwatched = false;
   }
   if (unwatched)
      return SUPERPAGE_UNWATCHED;
   // calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end   = (end_addr  >>PAGE_OFFSET_LENGTH);
   unwatched = true;
   for (ADDRESS i=page_number_start;i<=page_number_end;i++) {           // for each page, 
      if (superpage_unknown[i>>SECOND_LEVEL_PAGE_NUM_LENGTH]) {
         // update plb
         PLB_entry<ADDRESS> temp;
         temp.tag = i;
         temp.level = 'p';
         if (plb.check_and_update(temp))
            plb_misses++;
         // check watched or not
         if ((target_flags & WA_READ) && pt_read_watched[i])
            return PAGETABLE_WATCHED;
         if ((target_flags & WA_WRITE) && pt_write_watched[i])
            return PAGETABLE_WATCHED;
         if (pt_unknown[i])
            unwatched = false;
      }
   }
   if (unwatched)
      return PAGETABLE_UNWATCHED;
   ADDRESS plb_line_start = (start_addr>>PLB_LINE_OFFSET_LENGTH);
   ADDRESS plb_line_end   = (end_addr  >>PLB_LINE_OFFSET_LENGTH);
   for (ADDRESS i=plb_line_start;i<=plb_line_end;i++) {
      if (pt_unknown[i>>(PAGE_OFFSET_LENGTH-PLB_LINE_OFFSET_LENGTH)) {
         // update plb
         PLB_entry<ADDRESS> temp;
         temp.tag = i;
         temp.level = 'b';
         if (plb.check_and_update(temp))
            plb_misses++;
      }
   }
   if (wp->general_fault(start_addr, end_addr, target_flags))
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
   // calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_start = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS superpage_number_end = (end_addr>>SUPERPAGE_OFFSET_LENGTH);
   unsigned int firstpage_bitmap_change = 0, 
                lastpage_bitmap_change = 0, 
                first_superpage_bitmap_change = 0, 
                last_superpage_bitmap_change = 0;
   if (page_number_start == page_number_end) {  // if in the same page
      // check if read watched
      if (target_flags & WA_READ) {
         if (check_page_level_consistency(page_number_start, WA_READ, true)) {
            pt_read_watched[page_number_start] = true;
            if (check_superpage_level_consistency(superpage_number_start, WA_READ, true)) {
               superpage_read_watched[superpage_number_start] = true;
               if (check_seg_reg_level_consistency(WA_READ, true))
                  seg_reg_read_watched = true;
               else {
                  seg_reg_read_watched = false;
                  seg_reg_unknown = true;
               }
            }
            else {
               superpage_read_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         else {
            pt_read_watched[page_number_start] = false;
            pt_unknown[page_number_start] = true;
         }
      }
      // check if write watched  (same code as above)
      if (target_flags & WA_WRITE) {
         if (check_page_level_consistency(page_number_start, WA_WRITE, true)) {
            pt_write_watched[page_number_start] = true;
            if (check_superpage_level_consistency(superpage_number_start, WA_WRITE, true)) {
               superpage_write_watched[superpage_number_start] = true;
               if (check_seg_reg_level_consistency(WA_WRITE, true))
                  seg_reg_write_watched = true;
               else {
                  seg_reg_write_watched = false;
                  seg_reg_unknown = true;
               }
            }
            else {
               superpage_write_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         else {
            pt_write_watched[page_number_start] = false;
            pt_unknown[page_number_start] = true;
         }
      }
      if (pt_unknown[page_number_start])
         return end_addr - start_addr + 1;
      else
         return 1;
   }
   else {   // if not in the same page
      // setting start pagetables
      if (target_flags & WA_READ) {
         if (check_page_level_consistency(page_number_start, WA_READ, true)) {
            pt_read_watched[page_number_start] = true;
         else {
            pt_read_watched[page_number_start] = false;
            pt_unknown[page_number_start] = true;
         }
      }
      if (target_flags & WA_WRITE) {
         if (check_page_level_consistency(page_number_start, WA_WRITE, true)) {
            pt_write_watched[page_number_start] = true;
         else {
            pt_write_watched[page_number_start] = false;
            pt_unknown[page_number_start] = true;
         }
      }
      if (pt_unknown[page_number_start])
         firstpage_bitmap_change = ((page_number_start+1)<<PAGE_OFFSET_LENGTH) - start_addr;
      else
         firstpage_bitmap_change = 1;
      // setting end pagetables
      if (target_flags & WA_READ) {
         if (check_page_level_consistency(page_number_end, WA_READ, true)) {
            pt_read_watched[page_number_end] = true;
         else {
            pt_read_watched[page_number_end] = false;
            pt_unknown[page_number_end] = true;
         }
      }
      if (target_flags & WA_WRITE) {
         if (check_page_level_consistency(page_number_end, WA_WRITE, true)) {
            pt_write_watched[page_number_end] = true;
         else {
            pt_write_watched[page_number_end] = false;
            pt_unknown[page_number_end] = true;
         }
      }
      if (pt_unknown[page_number_end])
         lastpage_bitmap_change = end_addr - (page_number_end<<PAGE_OFFSET_LENGTH) + 1;
      else
         lastpage_bitmap_change = 1;
      // setting all pagetables in the middle
      if (target_flags & WA_READ) {
         for (ADDRESS i=page_number_start+1;i!=page_number_end;i++)
            pt_read_watched[i] = true;
      }
      if (target_flags & WA_WRITE) {
         for (ADDRESS i=page_number_start+1;i!=page_number_end;i++)
            pt_write_watched[i] = true;
      }
      if (superpage_number_start == superpage_number_end) { // if in the same superpage
         // check if read watched
         if (target_flags & WA_READ) {
            if (check_superpage_level_consistency(superpage_number_start, WA_READ, true)) {
               superpage_read_watched[superpage_number_start] = true;
               if (check_seg_reg_level_consistency(WA_READ, true))
                  seg_reg_read_watched = true;
               else {
                  seg_reg_read_watched = false;
                  seg_reg_unknown = true;
               }
            }
            else {
               superpage_read_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         // check if write watched (the same code as above)
         if (target_flags & WA_WRITE) {
            if (check_superpage_level_consistency(superpage_number_start, WA_WRITE, true)) {
               superpage_write_watched[superpage_number_start] = true;
               if (check_seg_reg_level_consistency(WA_WRITE, true))
                  seg_reg_write_watched = true;
               else {
                  seg_reg_write_watched = false;
                  seg_reg_unknown = true;
               }
            }
            else {
               superpage_write_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         if (superpage_unknown[superpage_number_start])
            return page_number_end - page_number_start - 1 + firstpage_bitmap_change + lastpage_bitmap_change;
         else
            return 1;
      }
      else {   // if not in the same superpage
         // setting start superpage
         if (target_flags & WA_READ) {
            if (check_superpage_level_consistency(superpage_number_start, WA_READ, true)) {
               superpage_read_watched[superpage_number_start] = true;
            else {
               superpage_read_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         if (target_flags & WA_WRITE) {
            if (check_superpage_level_consistency(superpage_number_start, WA_WRITE, true)) {
               superpage_write_watched[superpage_number_start] = true;
            else {
               superpage_write_watched[superpage_number_start] = false;
               superpage_unknown[superpage_number_start] = true;
            }
         }
         if (superpage_unknown[superpage_number_start])
            first_superpage_bitmap_change = ((superpage_number_start+1)<<SECOND_LEVEL_PAGE_NUM_LENGTH) - page_number_start;
         else
            first_superpage_bitmap_change = 1;
         // setting end superpage (the same code as above)
         if (target_flags & WA_READ) {
            if (check_superpage_level_consistency(superpage_number_end, WA_READ, true)) {
               superpage_read_watched[superpage_number_end] = true;
            else {
               superpage_read_watched[superpage_number_end] = false;
               superpage_unknown[superpage_number_end] = true;
            }
         }
         if (target_flags & WA_WRITE) {
            if (check_superpage_level_consistency(superpage_number_end, WA_WRITE, true)) {
               superpage_write_watched[superpage_number_end] = true;
            else {
               superpage_write_watched[superpage_number_end] = false;
               superpage_unknown[superpage_number_end] = true;
            }
         }
         if (superpage_unknown[superpage_number_end])
            first_superpage_bitmap_change = page_number_end - (superpage_number_end<<SECOND_LEVEL_PAGE_NUM_LENGTH) + 1;
         else
            first_superpage_bitmap_change = 1;
         // set all superpages in the middle
         if (target_flags & WA_READ) {
            for (ADDRESS i=superpage_number_start+1;i!=superpage_number_end;i++)
               superpage_read_watched[i] = true;
         }
         if (target_flags & WA_WRITE) {
            for (ADDRESS i=superpage_number_start+1;i!=superpage_number_end;i++)
               superpage_write_watched[i] = true;
         }
         if (target_flags & WA_READ) {
            if (check_seg_reg_level_consistency(WA_READ, true))
               seg_reg_read_watched = true;
            else {
               seg_reg_read_watched = false;
               seg_reg_unknown = true;
            }
         }
         if (target_flags & WA_WRITE) {
            if (check_seg_reg_level_consistency(WA_WRITE, true))
               seg_reg_write_watched = true;
            else {
               seg_reg_write_watched = false;
               seg_reg_unknown = true;
            }
         }
         if (seg_reg_unknown)
            return firstpage_bitmap_change + lastpage_bitmap_change 
              + first_superpage_bitmap_change + last_superpage_bitmap_change 
              + superpage_number_end - superpage_number_start - 1;
         else
            return 1;
      }
   }
}

template<class ADDRESS, class FLAGS>
int PT2_byte_acu_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int page_change = 0, superpage_change = 0, total_change = 0;
   bool consistent;
   ADDRESS page_number = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS superpage_number = (start_addr>>SUPERPAGE_OFFSET_LENGTH);
   ADDRESS i=start_addr;
   do {
      if ( ((target_flags & WA_READ ) && !wp->general_fault(i, i, WA_READ )) 
        || ((target_flags & WA_WRITE) && !wp->general_fault(i, i, WA_WRITE)) )
         page_change++;                               // if unwatched, overwrite in bitmap
      if ( (i & (PAGE_SIZE-1)) == (PAGE_SIZE-1) ) {   // if page_end
         if (page_change) {
            consistent = true;
            if (target_flags & WA_READ) {
               if (check_page_level_consistency(page_number, WA_READ, false)) {
                  pt_read_watched[page_number] = false;
                  pt_unknown[page_number] = false;
               }
               else {
                  pt_read_watched[page_number] = false;
                  pt_unknown[page_number] = true;
                  consistent = false;
               }
            }
            if (target_flags & WA_WRITE) {
               if (check_page_level_consistency(page_number, WA_WRITE, false)) {
                  pt_write_watched[page_number] = false;
                  pt_unknown[page_number] = false;
               }
               else {
                  pt_write_watched[page_number] = false;
                  pt_unknown[page_number] = true;
                  consistent = false;
               }
            }
            if (consistent)
               superpage_change++;
            else
               superpage_change += page_change;
         }
         page_change = 0;
         page_number++;
      }
      if ( (i & ((1<<SUPERPAGE_OFFSET_LENGTH)-1)) == ((1<<SUPERPAGE_OFFSET_LENGTH)-1) ) {    // if superpage_end
         if (superpage_change) {
            consistent = true;
            if (target_flags & WA_READ) {
               if (check_superpage_level_consistency(superpage_number, WA_READ, false)) {
                  superpage_read_watched[superpage_number] = false;
                  superpage_unknown[superpage_number] = false;
               }
               else {
                  superpage_read_watched[superpage_number] = false;
                  superpage_unknown[superpage_number] = true;
                  consistent = false;
               }
            }
            if (target_flags & WA_WRITE) {
               if (check_superpage_level_consistency(superpage_number, WA_WRITE, false)) {
                  superpage_write_watched[superpage_number] = false;
                  superpage_unknown[superpage_number] = false;
               }
               else {
                  superpage_write_watched[superpage_number] = false;
                  superpage_unknown[superpage_number] = true;
                  consistent = false;
               }
            }
            if (consistent)
               total_change++;
            else
               total_change += superpage_change;
         }
         superpage_change = 0;
         superpage_number++;
      }
      i++;
   } while (i!=end_addr+1);
   if (page_change) {
      consistent = true;
      if (target_flags & WA_READ) {
         if (check_page_level_consistency(page_number, WA_READ, false)) {
            pt_read_watched[page_number] = false;
            pt_unknown[page_number] = false;
         }
         else {
            pt_read_watched[page_number] = false;
            pt_unknown[page_number] = true;
            consistent = false;
         }
      }
      if (target_flags & WA_WRITE) {
         if (check_page_level_consistency(page_number, WA_WRITE, false)) {
            pt_write_watched[page_number] = false;
            pt_unknown[page_number] = false;
         }
         else {
            pt_write_watched[page_number] = false;
            pt_unknown[page_number] = true;
            consistent = false;
         }
      }
      if (consistent)
         superpage_change++;
      else
         superpage_change += page_change;
   }
   if (superpage_change) {
      consistent = true;
      if (target_flags & WA_READ) {
         if (check_superpage_level_consistency(superpage_number, WA_READ, false)) {
            superpage_read_watched[superpage_number] = false;
            superpage_unknown[superpage_number] = false;
         }
         else {
            superpage_read_watched[superpage_number] = false;
            superpage_unknown[superpage_number] = true;
            consistent = false;
         }
      }
      if (target_flags & WA_WRITE) {
         if (check_superpage_level_consistency(superpage_number, WA_WRITE, false)) {
            superpage_write_watched[superpage_number] = false;
            superpage_unknown[superpage_number] = false;
         }
         else {
            superpage_write_watched[superpage_number] = false;
            superpage_unknown[superpage_number] = true;
            consistent = false;
         }
      }
      if (consistent)
         total_change++;
      else
         total_change += superpage_change;
   }
   if (total_change) {
      consistent = true;
      if (target_flags & WA_READ) {
         if (check_seg_reg_level_consistency(WA_READ, false)) {
            seg_reg_read_watched = false;
            seg_reg_unknown = false;
         }
         else {
            seg_reg_read_watched = false;
            seg_reg_unknown = true;
            consistent = false;
         }
      }
      if (target_flags & WA_WRITE) {
         if (check_seg_reg_level_consistency(WA_WRITE, false)) {
            seg_reg_write_watched = false;
            seg_reg_unknown = false;
         }
         else {
            seg_reg_write_watched = false;
            seg_reg_unknown = true;
            consistent = false;
         }
      }
      if (consistent)
         total_change = 1;
      else
         total_change += 1;
   }
   return total_change;
}

template<class ADDRESS, class FLAGS>
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_page_level_consistency(ADDRESS page_number, bool watched) {
   ADDRESS start = (page_number<<PAGE_OFFSET_LENGTH);
   ADDRESS end   = ((page_number+1)<<PAGE_OFFSET_LENGTH);
   for (ADDRESS i=start;i!=end;i++) {
      if (wp->watch_fault(i, i) != watched)
         return false;
   }
   return true;
}

template<class ADDRESS, class FLAGS>
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_superpage_level_consistency(ADDRESS superpage_number, bool watched) {
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
bool PT2_byte_acu_single<ADDRESS, FLAGS>::check_seg_reg_level_consistency(bool watched) {
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
