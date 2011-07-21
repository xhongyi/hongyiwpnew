#ifndef PLB_H_
#define PLB_H_

#define PLB_SIZE                 128
#define PLB_LINE_OFFSET_LENGTH   4

#include <deque>
using namespace std;

template<class ADDRESS>
struct PLB_entry {
   ADDRESS tag;   
   char    level; // 'a' for all_watched; 's' for superpage; 'p' for pagetable; 'b' for bitmap;
};

template<class ADDRESS>
class PLB{
public:
   PLB();
   ~PLB();
   // returns true on a hit; false on a miss
   void plb_shootdown(ADDRESS start_addr, ADDRESS end_addr);
   bool check_and_update(PLB_entry<ADDRESS> check_entry);
private:
   deque<PLB_entry<ADDRESS> > plb_data;
   bool cache_overflow();
   void cache_kickout();
};

#include "plb.cpp"
#endif /*PlB_H_*/
