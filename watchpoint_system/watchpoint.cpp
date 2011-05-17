#ifndef WATCHPOINT_CPP_
#define WATCHPOINT_CPP_

#include "watchpoint.h"

/*
 * operator that is used for a += b
 *    adding all statistics of b to a and then return the result to a
 */
inline statistics_t& operator +=(statistics_t &a, const statistics_t &b) { // not inline implementation will make it multiple definition
   a.checks += b.checks;
   a.oracle_faults += b.oracle_faults;
   a.sets += b.sets;
   a.updates += b.updates;
	#ifdef PAGE_TABLE_SINGLE
	a.page_table_faults += b.page_table_faults;
	#endif
	#ifdef PAGE_TABLE_MULTI
	a.multi_page_table_faults += b.multi_page_table_faults;
	#endif
	#ifdef PAGE_TABLE2_SINGLE
   a.superpage_miss += b.superpage_miss;
   a.superpage_faults += b.superpage_faults;
   a.superpage_hits += b.superpage_hits;
   a.highest_faults += b.highest_faults;
   a.highest_hits += b.highest_hits;
	#endif
   return a;
}

/*
 * operator that is used for c = a + b
 *    adding all statistics of a and b and then return the result
 */
inline statistics_t operator +(const statistics_t &a, const statistics_t &b) {
   statistics_t result;
   result.checks = a.checks + b.checks;
   result.oracle_faults = a.oracle_faults + b.oracle_faults;
   result.sets = a.sets + b.sets;
   result.updates = a.updates + b.updates;
	#ifdef PAGE_TABLE_SINGLE
	result.page_table_faults = a.page_table_faults + b.page_table_faults;
	#endif
	#ifdef PAGE_TABLE_MULTI
	result.multi_page_table_faults = a.multi_page_table_faults + b.multi_page_table_faults;
	#endif
	#ifdef PAGE_TABLE2_SINGLE
	result.superpage_miss = a.superpage_miss + b.superpage_miss;
	result.superpage_faults = a.superpage_faults + b.superpage_faults;
	result.superpage_hits = a.superpage_hits + b.superpage_hits;
	result.highest_faults = a.highest_faults + b.highest_faults;
	result.highest_hits = a.highest_hits + b.highest_hits;
	#endif
   return result;
}

/*
 * Constructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint() {
    emulate_hardware = true;
}

/*
 * Constructor that turns off hardware emulation
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint(bool do_emulate_hardware) {
    emulate_hardware = do_emulate_hardware;
}

/*
 * Destructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::~WatchPoint() {
}

/*
 * Starting a thread with thread_id
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::start_thread(int32_t thread_id) {
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter == statistics.end()) {                           // if thread_id is not active
      Oracle<ADDRESS, FLAGS> empty_oracle;
      /*
       * Initiating statistics
       */
      statistics_inactive_iter = statistics_inactive.find(thread_id);   // search for inactive thread_id
                                                                        // (just in case we start an inactive thread again, 
                                                                        //    which is actually impossible)
      if (statistics_inactive_iter == statistics_inactive.end())        // if not found
         statistics[thread_id] = clear_statistics();                    // initiate to zeros
      else {                                                            // if found
         statistics[thread_id] = statistics_inactive_iter->second;      // copy the original inactive data
         statistics_inactive.erase(statistics_inactive_iter);           // erase thread_id from inactive
      }
      /*
       * Initiating Oracle
       */
      oracle_wp[thread_id] = empty_oracle;
      /*
       * Initiating Page Table
       */
      if (emulate_hardware) {
#ifdef PAGE_TABLE
          PageTable1_single<ADDRESS, FLAGS> empty_page_table(oracle_wp.find(thread_id)->second);
          page_table_wp[thread_id] = empty_page_table;
#endif
#ifdef PAGE_TABLE2_SINGLE
          PageTable2_single<ADDRESS, FLAGS> empty_page_table2(page_table_wp[thread_id]);
          page_table2_wp[thread_id] = empty_page_table2;
#endif
      }
      return 0;                                                         // normal start: return 0
   }
   return -1;                                                           // abnormal start: starting active thread again, return -1
}

/*
 * Ending an active thread
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter != statistics.end()) {                           // if thread_id is active
      oracle_wp_iter = oracle_wp.find(thread_id);
      statistics_inactive[thread_id] = statistics_iter->second;         // move its statistics to inactive
      statistics.erase(statistics_iter);                                // remove it from active statistics
      oracle_wp.erase(oracle_wp_iter);                                  // remove its Oracle watchpoint data
      if (emulate_hardware) {
#ifdef PAGE_TABLE
          page_table_wp.erase(thread_id);
#endif
#ifdef PAGE_TABLE2_SINGLE
          page_table2_wp.erase(thread_id);
#endif
      }
      return 0;                                                         // normal end: return 0
   }
   return -1;                                                           // abnormal end: thread_id not found or inactive, return -1
}

/*
 * Printing all active thread_id's
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_threads(ostream &output) {
   output <<"Printing active threads: "<<endl;
   for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) { // for all active thread_id's
      output <<"Thread #"<<statistics_iter->first<<" is active."<<endl;                                 // print to std::output
   }
   return;
}

/*
 * reset all watchpoints and statistics (both active and inactive)
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset() {
   Oracle<ADDRESS, FLAGS> empty_oracle;
   statistics_t empty_statistics = clear_statistics();
   oracle_wp_iter = oracle_wp.begin();
   #ifdef PAGE_TABLE
   page_table_wp_iter = page_table_wp.begin();
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   page_table2_wp_iter = page_table2_wp.begin();
   #endif
   for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
                                                            // for all active thread_id's
      statistics_iter->second = empty_statistics;           // clear statistics
      oracle_wp_iter->second = empty_oracle;                // clear Oracle watchpoints
      if (emulate_hardware) {
#ifdef PAGE_TABLE
          PageTable1_single<ADDRESS, FLAGS> empty_page_table(oracle_wp_iter->second);
          page_table_wp_iter->second = empty_page_table;
          page_table_wp_iter++;
#endif
#ifdef PAGE_TABLE2_SINGLE
          PageTable2_single<ADDRESS, FLAGS> empty_page_table2(page_table_wp_iter->second);
          page_table2_wp_iter->second = empty_page_table2;
          page_table2_wp_iter++;
#endif
      }
      oracle_wp_iter++;                                     // Oracle should be of the same length as statistics
   }
   for (statistics_inactive_iter = statistics_inactive.begin();statistics_inactive_iter != statistics_inactive.end();statistics_inactive_iter++)
                                                            // for all inactive thread_id's
      statistics_inactive_iter->second = empty_statistics;  // clear inactive statistics
   return;
}

/*
 * reset all watchpoint and statistics (both active and inactive) of thread_id
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset(int32_t thread_id) {
   end_thread(thread_id);                                            // end thread_id
   statistics_inactive_iter = statistics_inactive.find(thread_id);
   if (statistics_inactive_iter != statistics_inactive.end())        // if found inactive
      statistics_inactive_iter->second = clear_statistics();         // clear inactive statistics
   start_thread(thread_id);                                          // then start a new one
   return;
}

/*
 * Check for r/w faults in thread_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::general_fault(ADDRESS start, ADDRESS end, int32_t thread_id, FLAGS target_flags, bool ignore_statistics) {
   /*
    * check for faults
    */
   oracle_wp_iter = oracle_wp.find(thread_id);
   if (oracle_wp_iter != oracle_wp.end()) {                                               // if thread_id found active
      bool oracle_fault = oracle_wp_iter->second.general_fault(start, end, target_flags); // check if Oracle fault
      /*
       * initializing variables
       */
#ifdef PAGE_TABLE_SINGLE
      bool page_table_fault = false;                                                      // initialize variables here
#endif
#ifdef PAGE_TABLE_MULTI
      bool multi_page_table_fault = page_table_wp[thread_id].watch_fault(start, end);     // check if fault in thread_id page_table
#endif
#ifdef PAGE_TABLE2_SINGLE
      int page_table2_fault = page_table2_wp[thread_id].watch_fault(start, end);
#endif
      /*
       * emulating hardware
       */
      if (emulate_hardware) {
#ifdef PAGE_TABLE_SINGLE                                                                  // check if fault if only one single_page_table
         for (page_table_wp_iter=page_table_wp.begin();page_table_wp_iter!=page_table_wp.end();page_table_wp_iter++) {
            if (page_table_wp_iter->second.watch_fault(start, end) ) {
               page_table_fault = true;
               break;
            }
         }
#endif
      }
      /*
       * update statistics
       */
      if (!ignore_statistics) {                                         // if not ignore statistics
         statistics_iter = statistics.find(thread_id);
         statistics_iter->second.checks++;                              // checks++
         if (oracle_fault)                                              // and if it is a Oracle fault
            statistics_iter->second.oracle_faults++;                    // oracle_fault++
         if (emulate_hardware) {
#ifdef PAGE_TABLE_SINGLE
            if (page_table_fault)                                       // if it is a page_table fault
               statistics_iter->second.page_table_faults++;             // page_table_fault++
#endif
#ifdef PAGE_TABLE_MULTI
            if (multi_page_table_fault)                                 // if it is a page_table fault
               statistics_iter->second.multi_page_table_faults++;       // page_table_fault++
#endif
#ifdef PAGE_TABLE2_SINGLE
            if (page_table2_fault == MISSED)
               statistics_iter->second.superpage_miss++;
            else if (page_table2_fault == SUPERPAGE_UNWATCHED)
               statistics_iter->second.superpage_hits++;
            else if (page_table2_fault == SUPERPAGE_WATCHED) {
               statistics_iter->second.superpage_hits++;
               statistics_iter->second.superpage_faults++;
            }
            else if (page_table2_fault == ALL_UNWATCHED)
               statistics_iter->second.highest_hits++;
            else if (page_table2_fault == ALL_WATCHED) {
               statistics_iter->second.highest_hits++;
               statistics_iter->second.highest_faults++;
            }
#endif
         }
      }
      return oracle_fault;                                              // return Oracle fault
   }
   return false;                                                        // return false if not found or inactive
}

/*
 * Check for r/w faults in thread_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::watch_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_fault(start, end, thread_id, WA_READ | WA_WRITE, ignore_statistics);
}

/*
 * Check for r faults in thread_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::read_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_fault(start, end, thread_id, WA_READ, ignore_statistics);
}

/*
 * Check for w faults in thread_id and update statistics if not ignored
 */
template<class ADDRESS, class FLAGS>
bool  WatchPoint<ADDRESS, FLAGS>::write_fault(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_fault(start, end, thread_id, WA_WRITE, ignore_statistics);
}

/*
 * Printing all existing watchpoints thread by thread (active only)
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_watchpoints(ostream &output) {
   output <<"Printing all existing watchpoints: "<<endl;
   for (oracle_wp_iter = oracle_wp.begin();oracle_wp_iter != oracle_wp.end();oracle_wp_iter++) {
      output <<"Watchpoints in thread #"<<oracle_wp_iter->first<<":"<<endl;
      oracle_wp_iter->second.watch_print(output);
   }
   output <<"Watchpoint print ends."<<endl;
}

template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::general_change(ADDRESS start, ADDRESS end, int32_t thread_id, string target_flags, bool ignore_statistics) {
   /*
    * analyzing target_flags
    */
   FLAGS add_flag = 0, rm_flag = 0;
   if (target_flags[0] == '1')
      add_flag |= WA_READ;
   if (target_flags[1] == '1')
      add_flag |= WA_WRITE;
   if (target_flags[0] == '0')
      rm_flag |= WA_READ;
   if (target_flags[1] == '0')
      rm_flag |= WA_WRITE;
   /*
    * handle watchpoints
    */
   int fault_count = 0, fault_count_multi = 0;
   oracle_wp_iter = oracle_wp.find(thread_id);
   if (oracle_wp_iter != oracle_wp.end()) {                                   // if thread_id found
      if (add_flag)
         oracle_wp_iter->second.add_watchpoint(start, end, add_flag);         // add watchpoints
      if (rm_flag)
         oracle_wp_iter->second.rm_watchpoint(start, end, rm_flag);           // rm watchpoints
      /*
       * emulating hardware
       */
      if (emulate_hardware) {
#ifdef PAGE_TABLE_SINGLE
         fault_count = count_faults(start, end);
#endif
#ifdef PAGE_TABLE_MULTI
         fault_count_multi = count_faults(start, end, thread_id);
#endif

#ifdef PAGE_TABLE
         page_table_wp[thread_id].add_watchpoint(start, end, add_flag);      // set page_table
         page_table_wp[thread_id].rm_watchpoint(start, end, rm_flag);        // set page_table
#endif
#ifdef PAGE_TABLE2_SINGLE
         page_table2_wp[thread_id].add_watchpoint(start, end, add_flag);     // set page_table
         page_table2_wp[thread_id].rm_watchpoint(start, end, rm_flag);       // set page_table
#endif

#ifdef PAGE_TABLE_SINGLE
         fault_count -= count_faults(start, end);
#endif
#ifdef PAGE_TABLE_MULTI
         fault_count_multi -= count_faults(start, end, thread_id);
#endif
      }
      /*
       * update statistics
       */
      if (!ignore_statistics) {                                               // if not ignored
         statistics_iter = statistics.find(thread_id);
         if (target_flags[0] != 'x' && target_flags[1] != 'x')
            statistics_iter->second.sets++;                                   // sets++
         else
            statistics_iter->second.updates++;                                // updates++
#ifdef PAGE_TABLE_SINGLE
         statistics_iter->second.fault_count += abs(fault_count);
#endif
#ifdef PAGE_TABLE_SINGLE
         statistics_iter->second.fault_count_multi += abs(fault_count_multi);
#endif
      }
      return 0;                                                               // normal set: return 0
   }
   return -1;                                                                 // abnormal set: thread_id not found, return -1
}

//set  11   (r/w)
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "11", ignore_statistics);
}

//set  10
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "10", ignore_statistics);
}

//set  01
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "01", ignore_statistics);
}

//set  00
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "00", ignore_statistics);
}

//update 1x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "1x", ignore_statistics);
}

//update x1
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "x1", ignore_statistics);
}

//update 0x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "0x", ignore_statistics);
}

//update x0
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
   return general_change(start, end, thread_id, "x0", ignore_statistics);
}

/*
 * Called by the other comparison functions
 */
template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::general_compare(int32_t thread_id1, int32_t thread_id2, FLAGS flag_thread1, FLAGS flag_thread2) {
   /*
    * search the two threads in oracle_wp
    */
   oracle_wp_iter = oracle_wp.find(thread_id1);
   oracle_wp_iter_2 = oracle_wp.find(thread_id2);
   /*
    * if both are found in oracle_wp
    */
   if (oracle_wp_iter != oracle_wp.end() && oracle_wp_iter_2 != oracle_wp.end() ) {
      watchpoint_t<ADDRESS, FLAGS> temp = oracle_wp_iter->second.start_traverse();  // start traversing through thread #1
      do {
         if (temp.flags & flag_thread1) {                                           // check only for required flags in thread #1
            if (oracle_wp_iter_2->second.general_fault(temp.start_addr, 
                                                       temp.end_addr, 
                                                       flag_thread2) )              // check for faults for required flags in thread #2
               return true;
         }
      } while (oracle_wp_iter->second.continue_traverse(temp) );
      /*
      for (oracle_wp_iter->second.wp_iter = oracle_wp_iter->second.wp.begin();
            oracle_wp_iter->second.wp_iter != oracle_wp_iter->second.wp.end();
            oracle_wp_iter->second.wp_iter++) {                            // for each watchpoint ranges in thread #1
         if (oracle_wp_iter->second.wp_iter->flags & flag_thread1) {       // check only for required flags in thread #1
            if (oracle_wp_iter_2->second.general_fault(oracle_wp_iter->second.wp_iter->start_addr, 
                                                       oracle_wp_iter->second.wp_iter->end_addr, 
                                                       flag_thread2) ) {   // check for faults for required flags in thread #2
               return true;
            }
         }
      }
      */
   }
   return false;
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_rr(int32_t thread_id1, int32_t thread_id2) {
   return general_compare(thread_id1, thread_id2, WA_READ, WA_READ);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_rw(int32_t thread_id1, int32_t thread_id2) {
   return general_compare(thread_id1, thread_id2, WA_READ, WA_WRITE);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_wr(int32_t thread_id1, int32_t thread_id2) {
   return general_compare(thread_id1, thread_id2, WA_WRITE, WA_READ);
}

template<class ADDRESS, class FLAGS>
bool WatchPoint<ADDRESS, FLAGS>::compare_ww(int32_t thread_id1, int32_t thread_id2) {
   return general_compare(thread_id1, thread_id2, WA_WRITE, WA_WRITE);
}

/*
 * Get statistics for thread_id
 */
template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::get_statistics(int32_t thread_id) {
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter != statistics.end()) {                        // if found active
      return statistics_iter->second;                                // return active statistics
   }
   else {
      statistics_inactive_iter = statistics_inactive.find(thread_id);
      if (statistics_inactive_iter != statistics_inactive.end()) {   // if found inactive
         return statistics_inactive_iter->second;                    // return inactive statistics
      }
   }
   return clear_statistics();                                        // if not found, return zero statistics
}

/*
 * Set thread_id's statistics to input if found (both active and inactive)
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_statistics(int32_t thread_id, statistics_t input) {
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter != statistics.end()) {                        // if found active
      statistics_iter->second = input;                               // set statistics = input
      return 0;                                                      // normal set: return 0
   }
   else {
      statistics_inactive_iter = statistics_inactive.find(thread_id);
      if (statistics_inactive_iter != statistics_inactive.end()) {   // if found inactive
         statistics_inactive_iter->second = input;                   // set statistics = input
         return 0;                                                   // normal set: return 0
      }
   }
   return -1;                                                        // abnormal set: tread_id not found, return -1
}

/*
 * Returns a zero statistics_t
 */
template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::clear_statistics() {
   statistics_t empty;
   empty.checks=0;
   empty.oracle_faults=0;
   empty.sets=0;
   empty.updates=0;
   #ifdef PAGE_TABLE_SINGLE
   empty.page_table_faults=0;
   #endif
   #ifdef PAGE_TABLE_MULTI
   empty.multi_page_table_faults=0;
   #endif
   return empty;
}

/*
 * Print statistics thread by thread
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(ostream &output, bool active) {
   if (active) {        // if active, only print active threads' statistics only
      output << "Printing statistics for all active threads: " << endl;
      for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
         output << "Thread " << statistics_iter->first << " statistics: " << endl;
         print_statistics(statistics_iter->second, output);
      }
   }
   else {               // if !active, print all threads' statistics
      output << "Printing statistics for all threads (both active and inactive): " << endl;
      statistics_iter = statistics.begin();
      statistics_inactive_iter = statistics_inactive.begin();
      while (statistics_iter != statistics.end() || statistics_inactive_iter != statistics_inactive.end()) {
         if (statistics_inactive_iter == statistics_inactive.end()) {
            output << "Active Thread " << statistics_iter->first << " statistics: " << endl;
            print_statistics(statistics_iter->second, output);
            statistics_iter++;
         }
         else if (statistics_iter == statistics.end()) {
            output << "Inactive Thread " << statistics_inactive_iter->first << " statistics: " << endl;
            print_statistics(statistics_inactive_iter->second, output);
            statistics_inactive_iter++;
         }
         else {
            if (statistics_iter->first <= statistics_inactive_iter->first) {
               output << "Active Thread " <<statistics_iter->first << " statistics: " << endl;
               print_statistics(statistics_iter->second, output);
               statistics_iter++;
            }
            else {
               output << "Inactive Thread " << statistics_inactive_iter->first << " statistics: " << endl;
               print_statistics(statistics_inactive_iter->second, output);
               statistics_inactive_iter++;
            }
         }
      }
   }
   output <<"End of statistics printing."<<endl;
   return;
}

/*
 * Print statistics (called by print_statistics(bool active) )
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(const statistics_t& to_print, ostream &output) {
   output << setw(45) << "Checks for watchpoint faults: " << to_print.checks << endl;
   output << setw(45) << "Watchpoint faults taken: " << to_print.oracle_faults << endl;
   output << setw(45) << "Watchpoint \'set\'s: " << to_print.sets << endl;
   output << setw(45) << "Watchpoint \'update\'s: " << to_print.updates << endl;
   #ifdef PAGE_TABLE_SINGLE
   output << setw(45) << "Page table (single) faults taken: " << to_print.page_table_faults <<endl;
   output << setw(45) << "Page table bitmap changes: " << to_print.fault_count <<endl;
   #endif
   #ifdef PAGE_TABLE_MULTI
   output << setw(45) << "Page table (multi) faults taken: " << to_print.multi_page_table_faults <<endl;
   output << setw(45) << "Page table bitmap changes: " << to_print.fault_count_multi <<endl;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   output << setw(45) << "2_level Page table (single) quick hit: " << to_print.highest_hits <<endl;
   output << setw(45) << "2_level Page table (single) quick fault: " << to_print.highest_faults <<endl;
   output << setw(45) << "2_level Page table (single) superpage hit: " << to_print.superpage_hits <<endl;
   output << setw(45) << "2_level Page table (single) superpage fault: " << to_print.superpage_faults <<endl;
   output << setw(45) << "2_level Page table (single) superpage miss: " << to_print.superpage_miss <<endl;
   #endif
   output <<endl;
   return;
}

#ifdef PAGE_TABLE_SINGLE
/*
 * if we have only a single page_table for all threads, 
 * we have to check for faults for all threads. 
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::count_faults(ADDRESS start, ADDRESS end) {
   ADDRESS page_number_start = ((start>>PAGE_OFFSET_LENGTH)<<PAGE_OFFSET_LENGTH);
   int fault_count=0;
   for (ADDRESS i=page_number_start;i<=end;i+=(1<<PAGE_OFFSET_LENGTH)) {
      for (page_table_wp_iter=page_table_wp.begin();page_table_wp_iter!=page_table_wp.end();page_table_wp_iter++) {
         if (page_table_wp_iter->second.watch_fault(i, i)) {
            fault_count++;
            break;
         }
      }
   }
   return fault_count;
}
#endif

#ifdef PAGE_TABLE_MULTI
/*
 * if we have different page_table each threads, 
 * we only check for faults in a certian thread. 
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::count_faults(ADDRESS start, ADDRESS end, int32_t thread_id) {
   ADDRESS page_number_start = ((start>>PAGE_OFFSET_LENGTH)<<PAGE_OFFSET_LENGTH);
   int fault_count=0;
   page_table_wp_iter = page_table_wp.find(thread_id);
   for (ADDRESS i=page_number_start;i<=end;i+=(1<<PAGE_OFFSET_LENGTH)) {
      if (page_table_wp_iter->second.watch_fault(i, i)) {
         fault_count++;
      }
   }
   return fault_count;
}
#endif

#endif /* WATCHPOINT_CPP_ */
