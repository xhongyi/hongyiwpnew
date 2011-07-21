#ifndef PLB_CPP_
#define PLB_CPP_

#include "plb.h"

template<class ADDRESS>
PLB<ADDRESS>::PLB() {
}

template<class ADDRESS>
PLB<ADDRESS>::~PLB() {
}

template<class ADDRESS>
void PLB<ADDRESS>::plb_shootdown(ADDRESS start_addr, ADDRESS end_addr)
{
   typename deque<PLB_entry<ADDRESS> >::iterator i;
   for (i = plb_data.begin(); i != plb_data.end();) {
      if (i->level == 'a') {
         i = plb_data.erase(i);
         continue;
      }
      else if (i->level == 's') {
         if (end_addr >= (i->tag<<SUPERPAGE_OFFSET_LENGTH) &&
               start_addr <= (((i->tag+1)<<SUPERPAGE_OFFSET_LENGTH)-1)) {
            i = plb_data.erase(i);
            continue;
         }
      }
      else if (i->level == 'p') {
         if (end_addr >= (i->tag<<PAGE_OFFSET_LENGTH) &&
               start_addr <= (((i->tag+1)<<PAGE_OFFSET_LENGTH)-1)) {
            i = plb_data.erase(i);
            continue;
         }
      }
      else {
        if (end_addr >= (i->tag<<PLB_LINE_OFFSET_LENGTH) &&
               start_addr <= (((i->tag+1)<<PLB_LINE_OFFSET_LENGTH)-1)) {
            i = plb_data.erase(i);
            continue;
         }
      }
      i++;
   }
}

template<class ADDRESS>
bool PLB<ADDRESS>::check_and_update(PLB_entry<ADDRESS> check_entry) {
   typename deque<PLB_entry<ADDRESS> >::iterator i;
   for (i=plb_data.begin();i!=plb_data.end();i++) {
      if ((i->tag == check_entry.tag) && (i->level == check_entry.level)) {
         plb_data.erase(i);
         plb_data.push_front(check_entry);
         return true;
      }
   }
   if (check_entry.level == 'a')
      plb_shootdown(0, (ADDRESS)-1);
   else if (check_entry.level == 's') {
      plb_shootdown(check_entry.tag<<SUPERPAGE_OFFSET_LENGTH,
            ((check_entry.tag+1)<<SUPERPAGE_OFFSET_LENGTH)-1);
   }
   else if (check_entry.level == 'p') {
      plb_shootdown(check_entry.tag<<PAGE_OFFSET_LENGTH,
            ((check_entry.tag+1)<<PAGE_OFFSET_LENGTH)-1);
   }
   else {
      plb_shootdown(check_entry.tag<<PLB_LINE_OFFSET_LENGTH,
            ((check_entry.tag+1)<<PLB_LINE_OFFSET_LENGTH)-1);
   }
   plb_data.push_front(check_entry);
   if (cache_overflow())
      cache_kickout();
   return false;
}

template<class ADDRESS>
bool PLB<ADDRESS>::cache_overflow() {
   return (plb_data.size() > PLB_SIZE);
}

template<class ADDRESS>
void PLB<ADDRESS>::cache_kickout() {
   plb_data.pop_back();
}

#endif /*PlB_CPP_*/
