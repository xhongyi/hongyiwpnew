#ifndef OFF_CBM_H_
#define OFF_CBM_H_

#define LOG_OFF_CBM_SIZE         12UL
#define OFF_CBM_SIZE             (1<<LOG_OFF_CBM_SIZE)   // = one page = 4KB
#define OFF_CBM_UPPER_THREASHOLD 16
#define OFF_CBM_LOWER_THREASHOLD 4

#include <deque>
#include "mem_tracker.h"
using namespace std;

template<class ADDRESS, class FLAGS>
class Offcbm{
public:
   Offcbm(Oracle<ADDRESS, FLAGS> *wp_ref);
   Offcbm();
   ~Offcbm();
   // for updating the wlb
   //    only call these function when we are sure the range is within the off-cbm region
   unsigned int  general_fault   (ADDRESS start_addr, ADDRESS end_addr);
   // special search function, search in offcbm_pages first and then the oracle
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address    (ADDRESS target_addr);
   // for updating the wlb
   //    only call these function when we are sure the range is within the off-cbm region
   unsigned int  wp_operation    (ADDRESS start_addr, ADDRESS end_addr);
   // for removing off-chip bitmap within a range
   void rm_offcbm       (ADDRESS start_addr, ADDRESS end_addr);
   void rm_offcbm       ();
   // only on a kickout, decide whether to switch a page to off-cbm or to range or doing nothing
   //    return value: 1=off-cbm; 2=range;
   unsigned int  kickout_dirty   (ADDRESS target_addr);
private:
   Oracle<ADDRESS, FLAGS>*                oracle_wp;
   deque<ADDRESS>                         offcbm_pages;
   MemTracker<ADDRESS>                    wlb;
   deque< watchpoint_t<ADDRESS, FLAGS> >  ret_deq;
};

#include "off_cbm.cpp"
#endif /*OFF_CBM_H_*/
