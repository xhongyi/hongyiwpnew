/*
 * oracle.h
 *
 *  Created on: Nov 18, 2010
 *      Author: xhongyi
 */

#ifndef ORACLE_WP_H_
#define ORACLE_WP_H_

#include <deque>
#include <iostream>
#include <iomanip>
#include <ostream>
#include <fstream>
#include <algorithm>
#include "virtual_wp.h"
#include "wp_data_struct.h"

using namespace std;

unsigned long long total_print_number;
#define MAX_PRINT_NUM 100000000

template<class ADDRESS, class FLAGS>
class Oracle : public Virtual_wp<ADDRESS, FLAGS> {
public:
   Oracle();
   ~Oracle();
/*
   void  add_watch   (ADDRESS start_addr, ADDRESS end_addr);
   void  add_read    (ADDRESS start_addr, ADDRESS end_addr);
   void  add_write   (ADDRESS start_addr, ADDRESS end_addr);

   void  rm_watch (ADDRESS start_addr, ADDRESS end_addr);
   void  rm_read  (ADDRESS start_addr, ADDRESS end_addr);
   void  rm_write (ADDRESS start_addr, ADDRESS end_addr);
*/
   int  watch_fault (ADDRESS start_addr, ADDRESS end_addr);
   int  read_fault  (ADDRESS start_addr, ADDRESS end_addr);
   int  write_fault (ADDRESS start_addr, ADDRESS end_addr);
   
   typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
      search_address    (ADDRESS target_addr);

   void  watch_print    (ostream &output = cout);

   void  wp_operation   (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags,
                         bool (*flag_test)(FLAGS &x, FLAGS &y), FLAGS (*flag_op)(FLAGS &x, FLAGS &y) );
   int  add_watchpoint  (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int  rm_watchpoint   (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   int  general_fault   (ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags);
   
   //traverse functions
   watchpoint_t<ADDRESS, FLAGS>& start_traverse();
   bool continue_traverse(watchpoint_t<ADDRESS, FLAGS>& watchpoint);
   // statistics for SSTs
   int get_size();
   long long sst_insertions;
   int max_size;
private:
   /*
    * wp       is the container to hold all the ranges for watchpoint structure.
    * wp_iter  is the iterator used in both add_watchpoint and rm_watchpoint.
    * pre_iter is used for getting the node right before wp_iter.
    * aft_iter is used for getting the node right after wp_iter.
    *
    * All these data are declared here is to save time by avoiding creating
    * and destroying them every time add_watchpoint and rm_watchpoint is called.
    */
   deque< watchpoint_t<ADDRESS, FLAGS> > wp;
   typename deque< watchpoint_t<ADDRESS, FLAGS> >::iterator wp_iter;
   typename deque< watchpoint_t<ADDRESS, FLAGS> >::iterator pre_iter;
   typename deque< watchpoint_t<ADDRESS, FLAGS> >::iterator aft_iter;
   //special wp_iter for traverse functions, don't use this for other functions
   typename deque< watchpoint_t<ADDRESS, FLAGS> >::iterator wp_iter_traverse;
};
/*
template<class ADDRESS, class FLAGS>
typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator
   search_address (ADDRESS start_addr, deque<watchpoint_t<ADDRESS, FLAGS> > &wp);
*/

template<class FLAGS>
bool flag_include(FLAGS container_flags, FLAGS target_flags);

template<class FLAGS>
bool flag_exclude(FLAGS container_flags, FLAGS target_flags);

template<class FLAGS>
FLAGS flag_union (FLAGS &x, FLAGS &y);

template<class FLAGS>
FLAGS flag_diff (FLAGS &x, FLAGS &y);

#include "oracle_wp.cpp"
#endif /* ORACLE_WP_H_ */
