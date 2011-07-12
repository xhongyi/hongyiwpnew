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
bool PLB<ADDRESS>::check_and_update(PLB_entry<ADDRESS> check_entry) {
   typename deque<PLB_entry<ADDRESS> >::iterator i;
   for (i=plb_data.begin();i!=plb_data.end();i++) {
      if ((i->tag == check_entry.tag) && (i->level == check_entry.level)) {
         plb_data.erase(i);
         plb_data.push_front(check_entry);
         return true;
      }
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
