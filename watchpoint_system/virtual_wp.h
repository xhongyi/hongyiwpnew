/*
 * virtual_wp.h
 *    the structure of all watchpoints
 *  Created on: May 20, 2011
 *      Author: luoyixin
 */

#ifndef VIRTUAL_WP_H_
#define VIRTUAL_WP_H_

#include <deque>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <fstream>
#include "wp_data_struct.h"

using namespace std;

template<class ADDRESS, class FLAGS>
class Virtual_wp {
public:
   Virtual_wp(){};
   ~Virtual_wp(){};
   virtual int  general_fault  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) = 0;
   virtual int  watch_fault (ADDRESS start_addr, ADDRESS end_addr) = 0;
   virtual int  read_fault  (ADDRESS start_addr, ADDRESS end_addr) = 0;
   virtual int  write_fault (ADDRESS start_addr, ADDRESS end_addr) = 0;

   virtual int  add_watchpoint (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) = 0;
   virtual int  rm_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) = 0;
};
#endif /* VIRTUAL_WP_H_ */
