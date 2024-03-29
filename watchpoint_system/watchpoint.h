/*
 * watchpoint.h
 *
 * Created on: May 4, 2011
 *  Author: luoyixin
 */

#ifndef WATCHPOINT_H_
#define WATCHPOINT_H_

// Turn on a page table PER THREAD
//#define PAGE_TABLE_SINGLE
// Turn on a single page table shared between ALL THREADS
//#define PAGE_TABLE_MULTI
// Turn on a multi-level page table PER THREAD
//#define PAGE_TABLE2_SINGLE
// Turn on a multi-level page tabled shared between ALL THREADS
//#define PAGE_TABLE2_MULTI
// Turn on a byte accurate system PER THREAD
//#define PT2_BYTE_ACU_SINGLE
// Turn on a byte accureate system shared between ALL THREADS
//#define PT2_BYTE_ACU_MULTI
//#define MEM_TRACKER
//#define RC_SINGLE
//#define RC_OCBM
#define RC_OFFCBM
//#define IWATCHER

#ifdef PAGE_TABLE2_MULTI
#define PAGE_TABLE_MULTI
#endif

#ifdef PAGE_TABLE_SINGLE
#define PAGE_TABLE      // common part of page_table_single and page_table_multi
#endif

#ifdef PAGE_TABLE_MULTI
#define PAGE_TABLE
#endif

#ifdef PAGE_TABLE2_SINGLE
#define PAGE_TABLE      // common part of page_table_single and page_table_multi
#endif

#ifdef PAGE_TABLE
#define PRINT_VM_TRACE // Do you want to print out a trace of backing store ops?
#endif

#ifdef MEM_TRACKER
#define PRINT_MEMTRACKER_TRACE
#endif

#ifdef IWATCHER
#define PRINT_iWATCHER_TRACE
#endif

#if defined(RC_SINGLE) || defined(RC_OCBM) || defined(RC_OFFCBM)
#define PRINT_RANGE_TRACE // Do you want to print out a trace of the backing store ops for the RC?
#endif

#if defined(PRINT_VM_TRACE) || defined(PRINT_RANGE_TRACE) || defined(PRINT_MEMTRACKER_TRACE) || defined(PRINT_iWATCHER_TRACE)
#define PRINT_TRACE
#endif

#ifdef PT2_BYTE_ACU_MULTI
#define ORACLE_MULTI
#endif

#include <stdint.h>     // contains int32_t
#include <algorithm>
#include <cstdlib>
#include <string>
#include <cassert>
#include <map>
#include "oracle_wp.h"

#ifdef ORACLE_MULTI
#include "oracle_multi.h"
#endif

#ifdef PAGE_TABLE
#include "page_table1_single.h"
#endif

#ifdef PAGE_TABLE_MULTI
#include "page_table1_multi.h"
#endif

#ifdef PAGE_TABLE2_SINGLE
#include "page_table2_single.h"
#else
#ifdef PAGE_TABLE2_MULTI
#include "page_table2_single.h"
#endif
#endif

#ifdef PT2_BYTE_ACU_SINGLE
#include "pt2_byte_acu_single.h"
#else
#ifdef PT2_BYTE_ACU_MULTI
#include "pt2_byte_acu_single.h"
#endif
#endif

#ifdef MEM_TRACKER
#include "mem_tracker.h"
#endif

#ifdef RC_SINGLE
#include "range_cache.h"
#endif

#ifdef RC_OCBM
#include "range_cache.h"
#endif

#ifdef RC_OFFCBM
#include "range_cache.h"
#endif

#ifdef IWATCHER
#include "iwatcher.h"
#endif

#define IGNORE_STATS true
#define STORE_STATS false

#define ACTIVE_ONLY true
#define INCLUDE_INACTIVE false

/*
 * Structure that contains all hardware emulation statistics for one thread
 */
struct statistics_t {
   long long checks;                   // total number of checks on faults
   long long oracle_faults;            // total number of times that the Oracle get faults
   long long sets;                     // total number of times the API sets a watchpoint
   long long updates;                  // total number of times the API updates a watchpoint
   long long write_size;               // total size of sets and updates
   long long sst_insertions;           // total number of data copies while doing SST insertions
   int max_size;                       // max size of the total watchpoint system
   long long sum_size;                 // the sum of all recorded sizes when updates and sets, 
                                       //    used for calculating avg_size
   #ifdef PAGE_TABLE_SINGLE
   long long page_table_faults;        // total number of times that the page_table get faults
   long long change_count;             // total number of page_table_watch_bit changes
   #endif
   #ifdef PAGE_TABLE_MULTI
   long long page_table_multi_faults;  // total number of times that the page_table get faults
   long long change_count_multi;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   long long superpage_all_watched;
   long long superpage_all_readonly;
   long long superpage_watched;
   long long superpage_readonly;
   long long superpage_page_watched;
   long long superpage_page_readonly;
   long long superpage_available;
   long long change_count2;            // total number of page_table_watch_bit changes
   #endif
   #ifdef PAGE_TABLE2_MULTI
   long long superpage_miss_faults_multi;    // total number of times that we get lowest level faults
   long long superpage_miss_multi;           // total number of times that we need to get to the bottom level
   long long superpage_faults_multi;         // total number of times that the superpage get faults
   long long superpage_hits_multi;           // total number of times that we get superpage hits
   long long highest_faults_multi;           // total number of times that we get faults immediately
   long long highest_hits_multi;             // total number of times that all memory are watched/unwatched
   long long change_count2_multi;            // total number of page_table_watch_bit changes
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   long long pt2_byte_acu_seg_reg_hits;
   long long pt2_byte_acu_seg_reg_faults;
   long long pt2_byte_acu_superpage_hits;
   long long pt2_byte_acu_superpage_faults;
   long long pt2_byte_acu_page_hits;
   long long pt2_byte_acu_page_faults;
   long long pt2_byte_acu_bitmap_faults;
   long long pt2_byte_acu_changes;
   long long pt2_byte_acu_plb_misses;
   long long pt2_byte_acu_plb_wasnt_there;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   long long pt2_byte_acu_multi_seg_reg_hits;
   long long pt2_byte_acu_multi_seg_reg_faults;
   long long pt2_byte_acu_multi_superpage_hits;
   long long pt2_byte_acu_multi_superpage_faults;
   long long pt2_byte_acu_multi_page_hits;
   long long pt2_byte_acu_multi_page_faults;
   long long pt2_byte_acu_multi_bitmap_faults;
   long long pt2_byte_acu_multi_changes;
   long long pt2_byte_acu_multi_plb_misses;
   long long pt2_byte_acu_multi_plb_wasnt_there;
   #endif
   #ifdef MEM_TRACKER
   long long mem_tracker_read_miss;
   long long mem_tracker_write_miss;
   long long mem_tracker_mem_accesses;
   long long mem_tracker_write_blocks;
   #endif
   #ifdef RC_SINGLE
   long long rc_read_hits;
   long long rc_read_miss;
   long long rc_write_hits;
   long long rc_write_miss;
   long long rc_backing_store_accesses;
   long long rc_kickout_dirties;
   long long rc_kickouts;
   long long rc_complex_updates;
   #endif
   #ifdef RC_OCBM
   long long rc_ocbm_read_hits;
   long long rc_ocbm_read_miss;
   long long rc_ocbm_write_hits;
   long long rc_ocbm_write_miss;
   long long rc_ocbm_backing_store_accesses;
   long long rc_ocbm_kickout_dirties;
   long long rc_ocbm_kickouts;
   long long rc_ocbm_complex_updates;
   #endif
   #ifdef RC_OFFCBM
   long long rc_offcbm_read_hits;
   long long rc_offcbm_read_miss;
   long long rc_offcbm_write_hits;
   long long rc_offcbm_write_miss;
   long long rc_offcbm_wlb_miss;
   long long rc_offcbm_wlb_miss_size;
   long long rc_offcbm_backing_store_accesses;
   long long rc_offcbm_kickout_dirties;
   long long rc_offcbm_kickouts;
   long long rc_offcbm_complex_updates;
   long long rc_offcbm_offcbm_switch;
   long long rc_offcbm_range_switch;
   #endif
   #ifdef IWATCHER
   long long iwatcher_filter_miss;
   long long iwatcher_filter_miss_size;
   long long iwatcher_vm_changes;
   long long iwatcher_victim_kickouts;
   long long iwatcher_large_sets;
   #endif
};

/*
 * operator that is used for a += b
 */
inline statistics_t& operator +=(statistics_t &a, const statistics_t &b);

/*
 * operator that is used for c = a + b
 *    adding all statistics of a and b and then return the result
 */
inline statistics_t operator +(const statistics_t &a, const statistics_t &b);

/*
 * WatchPoint Library API
 */
template<class ADDRESS, class FLAGS>
class WatchPoint {
public:
   WatchPoint();              // Constructor
   WatchPoint(bool);          // Constructor that turns off hardware emulation.
   WatchPoint(ostream&);      // Constructor that enables trace outputting.
   ~WatchPoint();             // Destructor
   
   //Threading Calls
   int start_thread(int32_t thread_id);         // returns 0 on success, -1 on failure
                                                //    (can't have multiple active threads holding the same thread_id)
   int end_thread(int32_t thread_id);           // turns an active thread into inactive state
   void print_threads(ostream &output = cout);  // prints all active thread_id's
   
   //Reset Functions
   void reset();                                // resets all threads, whether active or inactive
   void reset(int32_t thread_id);               // resets thread #thread_id if found, whether active or inactive
   
   //Checking Watchpoints
   bool general_fault   (ADDRESS start, ADDRESS end, int32_t thread_id, FLAGS target_flags, bool ignore_statistics=false);
                                                                                                         // check for r/w faults
   bool watch_fault     (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   // check for r&w faults
   bool read_fault      (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   // only check r faults
   bool write_fault     (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   // only check w faults
   void print_watchpoints(ostream &output = cout);
   
   //Changing Watchpoints
   int general_change   (ADDRESS start, ADDRESS end, int32_t thread_id, string target_flags, bool ignore_statistics=false);
                                                                                                         //called by the following 8 functions
   int set_watch        (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //set    11 (rw)
   int set_read         (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //set    10
   int set_write        (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //set    01
   int rm_watch         (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //set    00
   
   int update_set_read  (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //update 1x
   int update_set_write (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //update x1
   int rm_read          (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //update 0x
   int rm_write         (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //update x0
   
   //Watchpoint Comparison Functions
   /*
    * Check every flag_thread1 watchpoints in thread_id1 and
    *       every flag_thread2 watchpoints in thread_id2
    * to see if any overlap exists.
    */
   bool general_compare (int32_t thread_id1, int32_t thread_id2, FLAGS flag_thread1, FLAGS flag_thread2);
   bool compare_rr      (int32_t thread_id1, int32_t thread_id2);
   bool compare_rw      (int32_t thread_id1, int32_t thread_id2);
   bool compare_wr      (int32_t thread_id1, int32_t thread_id2);
   bool compare_ww      (int32_t thread_id1, int32_t thread_id2);
   
   //Hardware Emulation Statistic Functions
   statistics_t get_statistics   (int32_t thread_id);
   statistics_t update_active_stats (int32_t thread_id);
   int          set_statistics   (int32_t thread_id, statistics_t input);
   statistics_t clear_statistics ();
   void         print_statistics (ostream &output = cout, bool active = false);
   void         print_statistics (const statistics_t& to_print, ostream &output = cout);
   void         print_size       (int32_t thread_id, ostream &output = cout);
   
private:
   //Data Structures (mainly maps from thread_id's to the thread's data)
   map<int32_t, Oracle<ADDRESS, FLAGS>* >                   oracle_wp;           // for storing byte accurate emulation class
   map<int32_t, statistics_t>                               statistics;          // for storing all active threads and statistics
   map<int32_t, statistics_t>                               statistics_inactive; // for storing all inactive threads and statistics (no overlaps)
   #ifdef ORACLE_MULTI
   Oracle_multi<ADDRESS, FLAGS>*                            oracle_multi;
   #endif
   #ifdef PAGE_TABLE
   map<int32_t, PageTable1_single<ADDRESS, FLAGS>* >        page_table_wp;       // for storing bitmaps of page_table watch_bits of each threads
   #endif                                                                        //    to fake a page_table watchpoint system
   #ifdef PAGE_TABLE_MULTI
   PageTable1_multi<ADDRESS, FLAGS>*                        page_table_multi;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   map<int32_t, PageTable2_single<ADDRESS, FLAGS>* >        page_table2_wp;      // for storing bitmaps of page_table watch_bits of each threads
   #endif                                                                        //    to fake a page_table watchpoint system
   #ifdef PAGE_TABLE2_MULTI
   PageTable2_single<ADDRESS, FLAGS>*                       page_table2_multi;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   map<int32_t, PT2_byte_acu_single<ADDRESS, FLAGS>* >      pt2_byte_acu;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   PT2_byte_acu_single<ADDRESS, FLAGS>*                     pt2_byte_acu_multi;
   #endif
   #ifdef MEM_TRACKER
   map<int32_t, MemTracker<ADDRESS>*>                       mem_tracker;
   #endif
   #ifdef RC_SINGLE
   map<int32_t, RangeCache<ADDRESS, FLAGS>* >               range_cache;
   #endif
   #ifdef RC_OCBM
   map<int32_t, RangeCache<ADDRESS, FLAGS>* >               range_cache_ocbm;
   #endif
   #ifdef RC_OFFCBM
   map<int32_t, RangeCache<ADDRESS, FLAGS>* >               range_cache_offcbm;
   #endif
   #ifdef IWATCHER
   IWatcher<ADDRESS, FLAGS>*                                iwatcher;
   #endif

   bool                                                     emulate_hardware;
   ostream                                                  &trace_output;
   void         print_trace (char command, int thread_id, unsigned int starter, unsigned int ender);
   
   //Useful iterators
   typename map<int32_t, Oracle<ADDRESS, FLAGS>* >::iterator oracle_wp_iter;
   typename map<int32_t, Oracle<ADDRESS, FLAGS>* >::iterator oracle_wp_iter_2;
   map<int32_t, statistics_t>::iterator                      statistics_iter;
   map<int32_t, statistics_t>::iterator                      statistics_inactive_iter;
   #ifdef PAGE_TABLE
   typename map<int32_t, PageTable1_single<ADDRESS, FLAGS>* >::iterator   page_table_wp_iter;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   typename map<int32_t, PageTable2_single<ADDRESS, FLAGS>* >::iterator   page_table2_wp_iter;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   typename map<int32_t, PT2_byte_acu_single<ADDRESS, FLAGS>* >::iterator pt2_byte_acu_iter;
   #endif
   #ifdef MEM_TRACKER
   typename map<int32_t, MemTracker<ADDRESS>*>::iterator                mem_tracker_iter;
   #endif
   #ifdef RC_SINGLE
   typename map<int32_t, RangeCache<ADDRESS, FLAGS>* >::iterator          range_cache_iter;
   #endif
   #ifdef RC_OCBM
   typename map<int32_t, RangeCache<ADDRESS, FLAGS>* >::iterator          range_cache_ocbm_iter;
   #endif
   #ifdef RC_OFFCBM
   typename map<int32_t, RangeCache<ADDRESS, FLAGS>* >::iterator          range_cache_offcbm_iter;
   #endif
};

#include "watchpoint.cpp"
#endif /* WATCHPOINT_H_ */
