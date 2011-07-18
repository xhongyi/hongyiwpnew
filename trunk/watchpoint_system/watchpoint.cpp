#ifndef WATCHPOINT_CPP_
#define WATCHPOINT_CPP_

#include <assert.h>
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
   a.write_size += b.write_size;
   a.sst_insertions += b.sst_insertions;
   a.max_size = max(a.max_size, b.max_size);
   a.sum_size += b.sum_size;
   #ifdef PAGE_TABLE_SINGLE
   a.page_table_faults += b.page_table_faults;
   a.change_count += b.change_count;
   #endif
   #ifdef PAGE_TABLE_MULTI
   a.page_table_multi_faults += b.page_table_multi_faults;
   a.change_count_multi += b.change_count_multi;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   a.superpage_miss += b.superpage_miss_faults;
   a.superpage_miss += b.superpage_miss;
   a.superpage_faults += b.superpage_faults;
   a.superpage_hits += b.superpage_hits;
   a.highest_faults += b.highest_faults;
   a.highest_hits += b.highest_hits;
   a.change_count2 += b.change_count2;
   #endif
   #ifdef PAGE_TABLE2_MULTI
   a.superpage_miss_multi += b.superpage_miss_faults_multi;
   a.superpage_miss_multi += b.superpage_miss_multi;
   a.superpage_faults_multi += b.superpage_faults_multi;
   a.superpage_hits_multi += b.superpage_hits_multi;
   a.highest_faults_multi += b.highest_faults_multi;
   a.highest_hits_multi += b.highest_hits_multi;
   a.change_count2_multi += b.change_count2_multi;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   a.pt2_byte_acu_seg_reg_hits += b.pt2_byte_acu_seg_reg_hits;
   a.pt2_byte_acu_seg_reg_faults += b.pt2_byte_acu_seg_reg_faults;
   a.pt2_byte_acu_superpage_hits += b.pt2_byte_acu_superpage_hits;
   a.pt2_byte_acu_superpage_faults += b.pt2_byte_acu_superpage_faults;
   a.pt2_byte_acu_page_hits += b.pt2_byte_acu_page_hits;
   a.pt2_byte_acu_page_faults += b.pt2_byte_acu_page_faults;
   a.pt2_byte_acu_bitmap_faults += b.pt2_byte_acu_bitmap_faults;
   a.pt2_byte_acu_changes += b.pt2_byte_acu_changes;
   a.pt2_byte_acu_plb_misses += b.pt2_byte_acu_plb_misses;
   a.pt2_byte_acu_plb_wasnt_there += b.pt2_byte_acu_plb_wasnt_there;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   a.pt2_byte_acu_multi_seg_reg_hits += b.pt2_byte_acu_multi_seg_reg_hits;
   a.pt2_byte_acu_multi_seg_reg_faults += b.pt2_byte_acu_multi_seg_reg_faults;
   a.pt2_byte_acu_multi_superpage_hits += b.pt2_byte_acu_multi_superpage_hits;
   a.pt2_byte_acu_multi_superpage_faults += b.pt2_byte_acu_multi_superpage_faults;
   a.pt2_byte_acu_multi_page_hits += b.pt2_byte_acu_multi_page_hits;
   a.pt2_byte_acu_multi_page_faults += b.pt2_byte_acu_multi_page_faults;
   a.pt2_byte_acu_multi_bitmap_faults += b.pt2_byte_acu_multi_bitmap_faults;
   a.pt2_byte_acu_multi_changes += b.pt2_byte_acu_multi_changes;
   a.pt2_byte_acu_multi_plb_misses += b.pt2_byte_acu_multi_plb_misses;
   a.pt2_byte_acu_multi_plb_wasnt_there += b.pt2_byte_acu_multi_plb_wasnt_there;
   #endif
   #ifdef MEM_TRACKER
   a.mem_tracker_read_miss += b.mem_tracker_read_miss;
   a.mem_tracker_write_miss += b.mem_tracker_write_miss;
   a.mem_tracker_mem_accesses += b.mem_tracker_mem_accesses;
   a.mem_tracker_write_blocks += b.mem_tracker_write_blocks;
   #endif
   #ifdef RC_SINGLE
   a.rc_read_hits += b.rc_read_hits;
   a.rc_read_miss += b.rc_read_miss;
   a.rc_write_hits += b.rc_write_hits;
   a.rc_write_miss += b.rc_write_miss;
   a.rc_backing_store_accesses += b.rc_backing_store_accesses;
   a.rc_kickout_dirties += b.rc_kickout_dirties;
   a.rc_kickouts += b.rc_kickouts;
   a.rc_complex_updates += b.rc_complex_updates;
   #endif
   #ifdef RC_OCBM
   a.rc_ocbm_read_hits += b.rc_ocbm_read_hits;
   a.rc_ocbm_read_miss += b.rc_ocbm_read_miss;
   a.rc_ocbm_write_hits += b.rc_ocbm_write_hits;
   a.rc_ocbm_write_miss += b.rc_ocbm_write_miss;
   a.rc_ocbm_backing_store_accesses += b.rc_ocbm_backing_store_accesses;
   a.rc_ocbm_kickout_dirties += b.rc_ocbm_kickout_dirties;
   a.rc_ocbm_kickouts += b.rc_ocbm_kickouts;
   a.rc_ocbm_complex_updates += b.rc_ocbm_complex_updates;
   #endif
   #ifdef RC_OFFCBM
   a.rc_offcbm_read_hits += b.rc_offcbm_read_hits;
   a.rc_offcbm_read_miss += b.rc_offcbm_read_miss;
   a.rc_offcbm_write_hits += b.rc_offcbm_write_hits;
   a.rc_offcbm_write_miss += b.rc_offcbm_write_miss;
   a.rc_offcbm_wlb_miss += b.rc_offcbm_wlb_miss;
   a.rc_offcbm_wlb_miss_size += b.rc_offcbm_wlb_miss_size;
   a.rc_offcbm_backing_store_accesses += b.rc_offcbm_backing_store_accesses;
   a.rc_offcbm_kickout_dirties += b.rc_offcbm_kickout_dirties;
   a.rc_offcbm_kickouts += b.rc_offcbm_kickouts;
   a.rc_offcbm_complex_updates += b.rc_offcbm_complex_updates;
   a.rc_offcbm_offcbm_switch += b.rc_offcbm_offcbm_switch;
   a.rc_offcbm_range_switch += b.rc_offcbm_range_switch;
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
   result.write_size = a.write_size + b.write_size;
   result.sst_insertions = a.sst_insertions + b.sst_insertions;
   result.max_size = max(a.max_size, b.max_size);
   result.sum_size = a.sum_size + b.sum_size;
   #ifdef PAGE_TABLE_SINGLE
   result.page_table_faults = a.page_table_faults + b.page_table_faults;
   result.change_count = a.change_count + b.change_count;
   #endif
   #ifdef PAGE_TABLE_MULTI
   result.page_table_multi_faults = a.page_table_multi_faults + b.page_table_multi_faults;
   result.change_count_multi = a.change_count_multi + b.change_count_multi;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   result.superpage_miss_faults = a.superpage_miss_faults + b.superpage_miss_faults;
   result.superpage_miss = a.superpage_miss + b.superpage_miss;
   result.superpage_faults = a.superpage_faults + b.superpage_faults;
   result.superpage_hits = a.superpage_hits + b.superpage_hits;
   result.highest_faults = a.highest_faults + b.highest_faults;
   result.highest_hits = a.highest_hits + b.highest_hits;
   result.change_count2 = a.change_count2 + b.change_count2;
   #endif
   #ifdef PAGE_TABLE2_MULTI
   result.superpage_miss_faults_multi = a.superpage_miss_faults_multi + b.superpage_miss_faults_multi;
   result.superpage_miss_multi = a.superpage_miss_multi + b.superpage_miss_multi;
   result.superpage_faults_multi = a.superpage_faults_multi + b.superpage_faults_multi;
   result.superpage_hits_multi = a.superpage_hits_multi + b.superpage_hits_multi;
   result.highest_faults_multi = a.highest_faults_multi + b.highest_faults_multi;
   result.highest_hits_multi = a.highest_hits_multi + b.highest_hits_multi;
   result.change_count2_multi = a.change_count2_multi + b.change_count2_multi;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   result.pt2_byte_acu_seg_reg_hits = a.pt2_byte_acu_seg_reg_hits + b.pt2_byte_acu_seg_reg_hits;
   result.pt2_byte_acu_seg_reg_faults = a.pt2_byte_acu_seg_reg_faults + b.pt2_byte_acu_seg_reg_faults;
   result.pt2_byte_acu_superpage_hits = a.pt2_byte_acu_superpage_hits + b.pt2_byte_acu_superpage_hits;
   result.pt2_byte_acu_superpage_faults = a.pt2_byte_acu_superpage_faults + b.pt2_byte_acu_superpage_faults;
   result.pt2_byte_acu_page_hits = a.pt2_byte_acu_page_hits + b.pt2_byte_acu_page_hits;
   result.pt2_byte_acu_page_faults = a.pt2_byte_acu_page_faults + b.pt2_byte_acu_page_faults;
   result.pt2_byte_acu_bitmap_faults = a.pt2_byte_acu_bitmap_faults + b.pt2_byte_acu_bitmap_faults;
   result.pt2_byte_acu_changes = a.pt2_byte_acu_changes + b.pt2_byte_acu_changes;
   result.pt2_byte_acu_plb_misses = a.pt2_byte_acu_plb_misses + b.pt2_byte_acu_plb_misses;
   result.pt2_byte_acu_plb_wasnt_there = a.pt2_byte_acu_plb_wasnt_there + b.pt2_byte_acu_plb_wasnt_there;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   result.pt2_byte_acu_multi_seg_reg_hits = a.pt2_byte_acu_multi_seg_reg_hits + b.pt2_byte_acu_multi_seg_reg_hits;
   result.pt2_byte_acu_multi_seg_reg_faults = a.pt2_byte_acu_multi_seg_reg_faults + b.pt2_byte_acu_multi_seg_reg_faults;
   result.pt2_byte_acu_multi_superpage_hits = a.pt2_byte_acu_multi_superpage_hits + b.pt2_byte_acu_multi_superpage_hits;
   result.pt2_byte_acu_multi_superpage_faults = a.pt2_byte_acu_multi_superpage_faults + b.pt2_byte_acu_multi_superpage_faults;
   result.pt2_byte_acu_multi_page_hits = a.pt2_byte_acu_multi_page_hits + b.pt2_byte_acu_multi_page_hits;
   result.pt2_byte_acu_multi_page_faults = a.pt2_byte_acu_multi_page_faults + b.pt2_byte_acu_multi_page_faults;
   result.pt2_byte_acu_multi_bitmap_faults = a.pt2_byte_acu_multi_bitmap_faults + b.pt2_byte_acu_multi_bitmap_faults;
   result.pt2_byte_acu_multi_changes = a.pt2_byte_acu_multi_changes + b.pt2_byte_acu_multi_changes;
   result.pt2_byte_acu_multi_plb_misses = a.pt2_byte_acu_multi_plb_misses + b.pt2_byte_acu_multi_plb_misses;
   result.pt2_byte_acu_multi_plb_wasnt_there = a.pt2_byte_acu_multi_plb_wasnt_there + b.pt2_byte_acu_multi_plb_wasnt_there;
   #endif
   #ifdef MEM_TRACKER
   result.mem_tracker_read_miss = a.mem_tracker_read_miss + b.mem_tracker_read_miss;
   result.mem_tracker_write_miss = a.mem_tracker_write_miss + b.mem_tracker_write_miss;
   result.mem_tracker_mem_accesses = a.mem_tracker_mem_accesses + b.mem_tracker_mem_accesses;
   result.mem_tracker_write_blocks = a.mem_tracker_write_blocks + b.mem_tracker_write_blocks;
   #endif
   #ifdef RC_SINGLE
   result.rc_read_hits = a.rc_read_hits + b.rc_read_hits;
   result.rc_read_miss = a.rc_read_miss + b.rc_read_miss;
   result.rc_write_hits = a.rc_write_hits + b.rc_write_hits;
   result.rc_write_miss = a.rc_write_miss + b.rc_write_miss;
   result.rc_backing_store_accesses = a.rc_backing_store_accesses + b.rc_backing_store_accesses;
   result.rc_kickout_dirties = a.rc_kickout_dirties + b.rc_kickout_dirties;
   result.rc_kickouts = a.rc_kickouts + b.rc_kickouts;
   result.rc_complex_updates = a.rc_complex_updates + b.rc_complex_updates;
   #endif
   #ifdef RC_OCBM
   result.rc_ocbm_read_hits = a.rc_ocbm_read_hits + b.rc_ocbm_read_hits;
   result.rc_ocbm_read_miss = a.rc_ocbm_read_miss + b.rc_ocbm_read_miss;
   result.rc_ocbm_write_hits = a.rc_ocbm_write_hits + b.rc_ocbm_write_hits;
   result.rc_ocbm_write_miss = a.rc_ocbm_write_miss + b.rc_ocbm_write_miss;
   result.rc_ocbm_backing_store_accesses = a.rc_ocbm_backing_store_accesses + b.rc_ocbm_backing_store_accesses;
   result.rc_ocbm_kickout_dirties = a.rc_ocbm_kickout_dirties + b.rc_ocbm_kickout_dirties;
   result.rc_ocbm_kickouts = a.rc_ocbm_kickouts + b.rc_ocbm_kickouts;
   result.rc_ocbm_complex_updates = a.rc_ocbm_complex_updates + b.rc_ocbm_complex_updates;
   #endif
   #ifdef RC_OFFCBM
   result.rc_offcbm_read_hits = a.rc_offcbm_read_hits + b.rc_offcbm_read_hits;
   result.rc_offcbm_read_miss = a.rc_offcbm_read_miss + b.rc_offcbm_read_miss;
   result.rc_offcbm_write_hits = a.rc_offcbm_write_hits + b.rc_offcbm_write_hits;
   result.rc_offcbm_write_miss = a.rc_offcbm_write_miss + b.rc_offcbm_write_miss;
   result.rc_offcbm_wlb_miss = a.rc_offcbm_wlb_miss + b.rc_offcbm_wlb_miss;
   result.rc_offcbm_wlb_miss_size = a.rc_offcbm_wlb_miss_size + b.rc_offcbm_wlb_miss_size;
   result.rc_offcbm_backing_store_accesses = a.rc_offcbm_backing_store_accesses + b.rc_offcbm_backing_store_accesses;
   result.rc_offcbm_kickout_dirties = a.rc_offcbm_kickout_dirties + b.rc_offcbm_kickout_dirties;
   result.rc_offcbm_kickouts = a.rc_offcbm_kickouts + b.rc_offcbm_kickouts;
   result.rc_offcbm_complex_updates = a.rc_offcbm_complex_updates + b.rc_offcbm_complex_updates;
   result.rc_offcbm_offcbm_switch = a.rc_offcbm_offcbm_switch + b.rc_offcbm_offcbm_switch;
   result.rc_offcbm_range_switch = a.rc_offcbm_range_switch + b.rc_offcbm_range_switch;
   #endif
   return result;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_trace(char command, int thread_id, unsigned int starter, unsigned int ender)
{
#ifdef PRINT_TRACE
   if(total_print_number < MAX_PRINT_NUM) {
       total_print_number++;
       trace_output << command << setw(5) << thread_id << setw(8) << starter << setw(8) << ender << endl;
   }
#endif
}

/*
 * Constructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint() :
    trace_output(std::cout)
{
   emulate_hardware = true;
#ifdef ORACLE_MULTI
   oracle_multi = new Oracle_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE_MULTI
   page_table_multi = new PageTable1_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE2_MULTI
   page_table2_multi = new PageTable2_single<ADDRESS, FLAGS>(page_table_multi);
#endif
#ifdef PT2_BYTE_ACU_MULTI
   pt2_byte_acu_multi = new PT2_byte_acu_single<ADDRESS, FLAGS>(oracle_multi);
#endif
}

/*
 * Constructor that turns off hardware emulation
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint(bool do_emulate_hardware) :
    trace_output(std::cout)
{
   emulate_hardware = do_emulate_hardware;
   if (emulate_hardware) {
#ifdef ORACLE_MULTI
      oracle_multi = new Oracle_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE_MULTI
      page_table_multi = new PageTable1_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE2_MULTI
      page_table2_multi = new PageTable2_single<ADDRESS, FLAGS>(page_table_multi);
#endif
#ifdef PT2_BYTE_ACU_MULTI
      pt2_byte_acu_multi = new PT2_byte_acu_single<ADDRESS, FLAGS>(oracle_multi);
#endif
   }
}

/*
 * Constructor that allows us to print backing store traces to a file.
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::WatchPoint(ostream &trace_file) :
    trace_output(trace_file)
{
    emulate_hardware=true;
#ifdef PRINT_TRACE
    trace_output << setfill('0');
    trace_output << hex;
#endif
#ifdef ORACLE_MULTI
    oracle_multi = new Oracle_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE_MULTI
    page_table_multi = new PageTable1_multi<ADDRESS, FLAGS>();
#endif
#ifdef PAGE_TABLE2_MULTI
    page_table2_multi = new PageTable2_single<ADDRESS, FLAGS>(page_table_multi);
#endif
#ifdef PT2_BYTE_ACU_MULTI
    pt2_byte_acu_multi = new PT2_byte_acu_single<ADDRESS, FLAGS>(oracle_multi);
#endif
}

/*
 * Destructor
 */
template<class ADDRESS, class FLAGS>
WatchPoint<ADDRESS, FLAGS>::~WatchPoint() {
   if (emulate_hardware) {
#ifdef ORACLE_MULTI
      delete oracle_multi;
#endif
#ifdef PAGE_TABLE_MULTI
      delete page_table_multi;
#endif
#ifdef PAGE_TABLE2_MULTI
      delete page_table2_multi;
#endif
#ifdef PT2_BYTE_ACU_MULTI
      delete pt2_byte_acu_multi;
#endif
   }
}

/*
 * Starting a thread with thread_id
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::start_thread(int32_t thread_id) {
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter == statistics.end()) {                           // if thread_id is not active
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
      oracle_wp[thread_id] = new Oracle<ADDRESS, FLAGS>;
      /*
       * Initiating Page Table
       */
      if (emulate_hardware) {
    if(emulate_hardware)
#ifdef ORACLE_MULTI
         oracle_multi->start_thread(thread_id, oracle_wp[thread_id]);
#endif
#ifdef PRINT_TRACE
         print_trace('i', thread_id, 0, 0);
#endif
#ifdef PAGE_TABLE
         page_table_wp[thread_id] = new PageTable1_single<ADDRESS, FLAGS>(oracle_wp.find(thread_id)->second);
#endif
#ifdef PAGE_TABLE_MULTI
         page_table_multi->start_thread(thread_id, page_table_wp[thread_id]);
#endif
#ifdef PAGE_TABLE2_SINGLE
         page_table2_wp[thread_id] = new PageTable2_single<ADDRESS, FLAGS>(page_table_wp.find(thread_id)->second);
#endif
#ifdef PT2_BYTE_ACU_SINGLE
         pt2_byte_acu[thread_id] = new PT2_byte_acu_single<ADDRESS, FLAGS>(oracle_wp.find(thread_id)->second);
#endif
#ifdef MEM_TRACKER
         mem_tracker[thread_id] = new MemTracker<ADDRESS>();
#endif
#ifdef RC_SINGLE
         range_cache[thread_id] = new RangeCache<ADDRESS, FLAGS>(trace_output, oracle_wp.find(thread_id)->second, thread_id);
#endif
#ifdef RC_OCBM
         range_cache_ocbm[thread_id] = new RangeCache<ADDRESS, FLAGS>(trace_output, oracle_wp.find(thread_id)->second, thread_id, true);
#endif
#ifdef RC_OFFCBM
         range_cache_offcbm[thread_id] = new RangeCache<ADDRESS, FLAGS>(trace_output, oracle_wp.find(thread_id)->second, thread_id, true, true);
#endif
      }
      return 0;                                                         // normal start: return 0
   }
   return -1;                                                           // abnormal start: starting active thread again, return -1
}

template<class ADDRESS, class FLAGS>
statistics_t WatchPoint<ADDRESS, FLAGS>::update_active_stats(int32_t thread_id) {
   statistics_t local_stats;
   statistics_iter = statistics.find(thread_id);
   if(statistics_iter != statistics.end()) {
      local_stats = statistics_iter->second;
      oracle_wp_iter = oracle_wp.find(thread_id);
      local_stats.sst_insertions += oracle_wp_iter->second->sst_insertions;
      local_stats.max_size = max(local_stats.max_size, oracle_wp_iter->second->max_size);
      if (emulate_hardware) {
#ifdef PT2_BYTE_ACU_SINGLE
         local_stats.pt2_byte_acu_plb_misses += pt2_byte_acu[thread_id]->plb_misses;
#endif
#ifdef PT2_BYTE_ACU_MULTI
         local_stats.pt2_byte_acu_multi_plb_misses += pt2_byte_acu_multi->plb_misses;
#endif
#ifdef RC_SINGLE
         range_cache_iter = range_cache.find(thread_id);
         local_stats.rc_kickout_dirties += range_cache_iter->second->kickout_dirty;
         local_stats.rc_kickouts += range_cache_iter->second->kickout;
         local_stats.rc_complex_updates += range_cache_iter->second->complex_updates;
#endif
#ifdef RC_OCBM
         range_cache_ocbm_iter = range_cache_ocbm.find(thread_id);
         local_stats.rc_ocbm_kickout_dirties += range_cache_ocbm_iter->second->kickout_dirty;
         local_stats.rc_ocbm_kickouts += range_cache_ocbm_iter->second->kickout;
         local_stats.rc_ocbm_complex_updates += range_cache_ocbm_iter->second->complex_updates;
#endif
#ifdef RC_OFFCBM
         range_cache_offcbm_iter = range_cache_offcbm.find(thread_id);
         local_stats.rc_offcbm_wlb_miss += range_cache_offcbm_iter->second->wlb_miss;
         local_stats.rc_offcbm_wlb_miss_size += range_cache_offcbm_iter->second->wlb_miss_size;
         local_stats.rc_offcbm_kickout_dirties += range_cache_offcbm_iter->second->kickout_dirty;
         local_stats.rc_offcbm_kickouts += range_cache_offcbm_iter->second->kickout;
         local_stats.rc_offcbm_complex_updates += range_cache_offcbm_iter->second->complex_updates;
         local_stats.rc_offcbm_offcbm_switch += range_cache_offcbm_iter->second->offcbm_switch;
         local_stats.rc_offcbm_range_switch += range_cache_offcbm_iter->second->range_switch;
#endif
      }
   }
   else
      local_stats = clear_statistics();
   return local_stats;
}

/*
 * Ending an active thread
 */
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   statistics_t local_stats = update_active_stats(thread_id);
   statistics_iter = statistics.find(thread_id);
   if (statistics_iter != statistics.end()) {                           // if thread_id is active
      if (emulate_hardware){
#ifdef ORACLE_MULTI
         oracle_multi->end_thread(thread_id);
#endif
      }
      oracle_wp_iter = oracle_wp.find(thread_id);
      delete oracle_wp_iter->second;
      oracle_wp.erase(oracle_wp_iter);                                  // remove its Oracle watchpoint data
      if (emulate_hardware) {
#ifdef PAGE_TABLE
         page_table_wp_iter = page_table_wp.find(thread_id);
         delete page_table_wp_iter->second;
         page_table_wp.erase(page_table_wp_iter);
#endif
#ifdef PAGE_TABLE_MULTI
         page_table_multi->end_thread(thread_id);
#endif
#ifdef PAGE_TABLE2_SINGLE
         page_table2_wp_iter = page_table2_wp.find(thread_id);
         delete page_table2_wp_iter->second;
         page_table2_wp.erase(page_table2_wp_iter);
#endif
#ifdef PAGE_TABLE2_MULTI
         page_table2_multi->rm_watchpoint(0, 0xffffffff);                     // check all pages
#endif
#ifdef PT2_BYTE_ACU_SINGLE
         pt2_byte_acu_iter = pt2_byte_acu.find(thread_id);
         delete pt2_byte_acu_iter->second;
         pt2_byte_acu.erase(pt2_byte_acu_iter);
#endif
#ifdef MEM_TRACKER
         mem_tracker_iter = mem_tracker.find(thread_id);
         delete mem_tracker_iter->second;
         mem_tracker.erase(mem_tracker_iter);
#endif
#ifdef RC_SINGLE
         range_cache_iter = range_cache.find(thread_id);
         delete range_cache_iter->second;
         range_cache.erase(range_cache_iter);
#endif
#ifdef RC_OCBM
         range_cache_ocbm_iter = range_cache_ocbm.find(thread_id);
         delete range_cache_ocbm_iter->second;
         range_cache_ocbm.erase(range_cache_ocbm_iter);
#endif
#ifdef RC_OFFCBM
         range_cache_offcbm_iter = range_cache_offcbm.find(thread_id);
         delete range_cache_offcbm_iter->second;
         range_cache_offcbm.erase(range_cache_offcbm_iter);
#endif
      }
      statistics_inactive[thread_id] = local_stats;                     // move its statistics to inactive
      statistics.erase(statistics_iter);                                // remove it from active statistics
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
      output <<"Thread #"<<statistics_iter->first<<" is active."<<endl;                               // print to std::output
   }
   return;
}

/*
 * reset all watchpoints and statistics (both active and inactive)
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::reset() {
   int32_t *active_thread_id = new int32_t[(unsigned int)statistics.size()];
   int i=0;
   for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
                                                            // for all active thread_id's
      active_thread_id[i] = statistics_iter->first;                        // call reset
      i++;
   }
   for (i=0;i<(int)statistics.size();i++) {
      reset(active_thread_id[i]);
   }
   for (statistics_inactive_iter = statistics_inactive.begin();statistics_inactive_iter != statistics_inactive.end();statistics_inactive_iter++)
                                                            // for all inactive thread_id's
      statistics_inactive_iter->second = clear_statistics();  // clear inactive statistics
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
      bool oracle_fault = oracle_wp_iter->second->general_fault(start, end, target_flags); // check if Oracle fault
      //bool oracle_fault = oracle_wp_iter->second->watch_fault(start, end);              // for checking pt2_byte_acu
      /*
       * declaring variables
       */
#ifdef PRINT_VM_TRACE
      bool print_me_vm = false;
#endif
#ifdef PRINT_RANGE_TRACE
      bool print_me_rc = false;
#endif
#ifdef PAGE_TABLE_SINGLE
      bool page_table_fault = false;
#endif
#ifdef PAGE_TABLE_MULTI
      bool page_table_multi_fault = false;
#endif
#ifdef PAGE_TABLE2_SINGLE
      unsigned int page_table2_fault = 0;
#endif
#ifdef PAGE_TABLE2_MULTI
      unsigned int page_table2_multi_fault = 0;
#endif
#ifdef PT2_BYTE_ACU_SINGLE
      unsigned int pt2_byte_acu_fault = 0;
      long long temp_pt2_byte_acu_misses = 0;
#endif
#ifdef PT2_BYTE_ACU_MULTI
      unsigned int pt2_byte_acu_multi_fault = 0;
      long long temp_pt2_byte_acu_multi_misses = 0;
#endif
#ifdef MEM_TRACKER
      unsigned int mem_tracker_read_mises = 0;
#endif
#ifdef RC_SINGLE
      unsigned int range_cache_misses = 0;
#endif
#ifdef RC_OCBM
      unsigned int range_cache_ocbm_misses = 0;
#endif
#ifdef RC_OFFCBM
      unsigned int range_cache_offcbm_misses = 0;
#endif
      /*
       * emulating hardware
       */
      if (emulate_hardware) {
          /*
           * initializing variables
           */
#ifdef PAGE_TABLE_SINGLE
         page_table_fault = page_table_wp[thread_id]->watch_fault(start, end);        // check if fault in thread_id page_table
#ifdef PRINT_VM_TRACE
         if(page_table_fault)
             print_me_vm = true;
#endif
#endif
#ifdef PAGE_TABLE_MULTI
         page_table_multi_fault = page_table_multi->watch_fault(start, end);
#ifdef PRINT_VM_TRACE
         if(page_table_multi_fault)
             print_me_vm = true;
#endif
#endif
#ifdef PAGE_TABLE2_SINGLE
         page_table2_fault = page_table2_wp[thread_id]->watch_fault(start, end);
#ifdef PRINT_VM_TRACE
         if(page_table2_fault)
             print_me_vm = true;
#endif
#endif
#ifdef PAGE_TABLE2_MULTI
         page_table2_multi_fault = page_table2_multi->watch_fault(start, end);
#ifdef PRINT_VM_TRACE
         if(page_table2_multi_fault)
             print_me_vm = true;
#endif
#endif
#ifdef PRINT_VM_TRACE
         if(print_me_vm)
             print_trace('k', thread_id, start, end);
#endif
#ifdef PT2_BYTE_ACU_SINGLE
         temp_pt2_byte_acu_misses = pt2_byte_acu[thread_id]->plb_misses;
         pt2_byte_acu_fault = pt2_byte_acu[thread_id]->general_fault(start, end, target_flags);
#endif
#ifdef PT2_BYTE_ACU_MULTI
         temp_pt2_byte_acu_multi_misses = pt2_byte_acu_multi->plb_misses;
         pt2_byte_acu_multi_fault = pt2_byte_acu_multi->general_fault(start, end, target_flags);
#endif
#ifdef MEM_TRACKER
         mem_tracker_read_mises = mem_tracker[thread_id]->general_fault(start, end);
#ifdef PRINT_MEMTRACKER_TRACE
         if(mem_tracker_read_mises)
             print_trace('k', thread_id, start, end);
#endif
#endif
#ifdef RC_SINGLE
         range_cache_misses = range_cache[thread_id]->watch_fault(start, end);
#ifdef PRINT_RANGE_TRACE
         if(range_cache_misses)
             print_me_rc = true;
#endif
#endif
#ifdef RC_OCBM
         range_cache_ocbm_misses = range_cache_ocbm[thread_id]->watch_fault(start, end);
#ifdef PRINT_RANGE_TRACE
         if(range_cache_ocbm_misses)
             print_me_rc = true;
#endif
#endif
#ifdef RC_OFFCBM
         range_cache_offcbm_misses = range_cache_offcbm[thread_id]->watch_fault(start, end);
#ifdef PRINT_RANGE_TRACE
         if(range_cache_offcbm_misses)
             print_me_rc = true;
#endif
#endif
#ifdef PRINT_RANGE_TRACE
         if(print_me_rc)
             print_trace('k', thread_id, start, end);
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
            if (page_table_multi_fault)                                 // if it is a page_table fault
               statistics_iter->second.page_table_multi_faults++;       // page_table_fault++
#endif
#ifdef PAGE_TABLE2_SINGLE
            if (page_table2_fault == PAGETABLE_UNWATCHED)
               statistics_iter->second.superpage_miss++;
            else if (page_table2_fault == PAGETABLE_WATCHED) {
               statistics_iter->second.superpage_miss++;
               statistics_iter->second.superpage_miss_faults++;
            }
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
#ifdef PAGE_TABLE2_MULTI
            if (page_table2_multi_fault == PAGETABLE_UNWATCHED)
               statistics_iter->second.superpage_miss_multi++;
            else if (page_table2_multi_fault == PAGETABLE_WATCHED) {
               statistics_iter->second.superpage_miss_multi++;
               statistics_iter->second.superpage_miss_faults_multi++;
            }
            else if (page_table2_multi_fault == SUPERPAGE_UNWATCHED)
               statistics_iter->second.superpage_hits_multi++;
            else if (page_table2_multi_fault == SUPERPAGE_WATCHED) {
               statistics_iter->second.superpage_hits_multi++;
               statistics_iter->second.superpage_faults_multi++;
            }
            else if (page_table2_multi_fault == ALL_UNWATCHED)
               statistics_iter->second.highest_hits_multi++;
            else if (page_table2_multi_fault == ALL_WATCHED) {
               statistics_iter->second.highest_hits_multi++;
               statistics_iter->second.highest_faults_multi++;
            }
#endif
#ifdef PT2_BYTE_ACU_SINGLE
            if (pt2_byte_acu_fault == BITMAP_WATCHED)
               statistics_iter->second.pt2_byte_acu_bitmap_faults++;
            else if (pt2_byte_acu_fault == PAGETABLE_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_page_hits++;
            else if (pt2_byte_acu_fault == PAGETABLE_WATCHED) {
               statistics_iter->second.pt2_byte_acu_page_hits++;
               statistics_iter->second.pt2_byte_acu_page_faults++;
            }
            else if (pt2_byte_acu_fault == SUPERPAGE_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_superpage_hits++;
            else if (pt2_byte_acu_fault == SUPERPAGE_WATCHED) {
               statistics_iter->second.pt2_byte_acu_superpage_hits++;
               statistics_iter->second.pt2_byte_acu_superpage_faults++;
            }
            else if (pt2_byte_acu_fault == ALL_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_seg_reg_hits++;
            else if (pt2_byte_acu_fault == ALL_WATCHED) {
               statistics_iter->second.pt2_byte_acu_seg_reg_hits++;
               statistics_iter->second.pt2_byte_acu_seg_reg_faults++;
            }
            if(temp_pt2_byte_acu_misses != pt2_byte_acu[thread_id]->plb_misses)
               statistics_iter->second.pt2_byte_acu_plb_wasnt_there++;
#endif
#ifdef PT2_BYTE_ACU_MULTI
            if (pt2_byte_acu_multi_fault == BITMAP_WATCHED)
               statistics_iter->second.pt2_byte_acu_multi_bitmap_faults++;
            else if (pt2_byte_acu_multi_fault == PAGETABLE_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_multi_page_hits++;
            else if (pt2_byte_acu_multi_fault == PAGETABLE_WATCHED) {
               statistics_iter->second.pt2_byte_acu_multi_page_hits++;
               statistics_iter->second.pt2_byte_acu_multi_page_faults++;
            }
            else if (pt2_byte_acu_multi_fault == SUPERPAGE_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_multi_superpage_hits++;
            else if (pt2_byte_acu_multi_fault == SUPERPAGE_WATCHED) {
               statistics_iter->second.pt2_byte_acu_multi_superpage_hits++;
               statistics_iter->second.pt2_byte_acu_multi_superpage_faults++;
            }
            else if (pt2_byte_acu_multi_fault == ALL_UNWATCHED)
               statistics_iter->second.pt2_byte_acu_multi_seg_reg_hits++;
            else if (pt2_byte_acu_multi_fault == ALL_WATCHED) {
               statistics_iter->second.pt2_byte_acu_multi_seg_reg_hits++;
               statistics_iter->second.pt2_byte_acu_multi_seg_reg_faults++;
            }
            if(temp_pt2_byte_acu_multi_misses != pt2_byte_acu_multi->plb_misses)
                statistics_iter->second.pt2_byte_acu_multi_plb_wasnt_there++;
#endif
#ifdef MEM_TRACKER
         if (mem_tracker_read_mises) {
            statistics_iter->second.mem_tracker_mem_accesses += mem_tracker_read_mises;
            statistics_iter->second.mem_tracker_read_miss++;
         }
#endif
#ifdef RC_SINGLE
            if (range_cache_misses) {
               statistics_iter->second.rc_read_miss++;
               statistics_iter->second.rc_backing_store_accesses += range_cache_misses;
            }
            else 
               statistics_iter->second.rc_read_hits++;
#endif
#ifdef RC_OCBM
            if (range_cache_ocbm_misses) {
               statistics_iter->second.rc_ocbm_read_miss++;
               statistics_iter->second.rc_ocbm_backing_store_accesses += range_cache_ocbm_misses;
            }
            else 
               statistics_iter->second.rc_ocbm_read_hits++;
#endif
#ifdef RC_OFFCBM
            if (range_cache_offcbm_misses) {
               statistics_iter->second.rc_offcbm_read_miss++;
               statistics_iter->second.rc_offcbm_backing_store_accesses += range_cache_offcbm_misses;
            }
            else 
               statistics_iter->second.rc_offcbm_read_hits++;
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
      oracle_wp_iter->second->watch_print(output);
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
#ifdef PAGE_TABLE
   unsigned int change_count = 0;
#endif
#ifdef PAGE_TABLE_MULTI
   unsigned int change_count_multi = 0;
#endif
#ifdef PAGE_TABLE2_SINGLE
   unsigned int change_count2 = 0;
#endif
#ifdef PAGE_TABLE2_MULTI
   unsigned int change_count2_multi = 0;
#endif
#ifdef PT2_BYTE_ACU_SINGLE
   unsigned int change_count2_byte_acu = 0;
#endif
#ifdef PT2_BYTE_ACU_MULTI
   unsigned int change_count2_byte_acu_multi = 0;
#endif
#ifdef MEM_TRACKER
   unsigned int mem_tracker_write_mises = 0;
#endif
#ifdef RC_SINGLE
   unsigned int range_cache_write_misses = 0;
#endif
#ifdef RC_OCBM
   unsigned int range_cache_ocbm_write_misses = 0;
#endif
#ifdef RC_OFFCBM
   unsigned int range_cache_offcbm_write_misses = 0;
#endif
   oracle_wp_iter = oracle_wp.find(thread_id);
   if (oracle_wp_iter != oracle_wp.end()) {                                   // if thread_id found
      if (add_flag)
         oracle_wp_iter->second->add_watchpoint(start, end, add_flag);         // add watchpoints
      if (rm_flag)
         oracle_wp_iter->second->rm_watchpoint(start, end, rm_flag);           // rm watchpoints
      /*
       * emulating hardware
       */
      if (emulate_hardware) {
#ifdef ORACLE_MULTI
         oracle_multi->add_watchpoint(start, end, add_flag);
         oracle_multi->rm_watchpoint(start, end, rm_flag);
#endif
#ifdef MEM_TRACKER
         mem_tracker_write_mises = mem_tracker[thread_id]->wp_operation(start, end);
#endif
         if (add_flag) {
#ifdef PAGE_TABLE
            change_count = page_table_wp[thread_id]->add_watchpoint(start, end);      // set page_table
#endif
#ifdef PAGE_TABLE_MULTI
            change_count_multi = page_table_multi->add_watchpoint(start, end);
#endif
#ifdef PAGE_TABLE2_SINGLE
            if (change_count)
               change_count2 = page_table2_wp[thread_id]->add_watchpoint(start, end);    // set page_table
#endif
#ifdef PAGE_TABLE2_MULTI
            if (change_count_multi)
               change_count2_multi = page_table2_multi->add_watchpoint(start, end);     // set page_table
#endif
#ifdef PT2_BYTE_ACU_SINGLE
            if (rm_flag) {
               pt2_byte_acu[thread_id]->add_watchpoint(start, end, add_flag);
               change_count2_byte_acu = pt2_byte_acu[thread_id]->rm_watchpoint(start, end, rm_flag);
               pt2_byte_acu[thread_id]->general_fault(start, end, add_flag | rm_flag);
            }
            else {
               change_count2_byte_acu = pt2_byte_acu[thread_id]->add_watchpoint(start, end, add_flag);
               pt2_byte_acu[thread_id]->general_fault(start, end, add_flag);
            }
#endif
#ifdef PT2_BYTE_ACU_MULTI
            if (rm_flag) {
               pt2_byte_acu_multi->add_watchpoint(start, end, add_flag);
               change_count2_byte_acu_multi = pt2_byte_acu_multi->add_watchpoint(start, end, rm_flag);
               pt2_byte_acu_multi->general_fault(start, end, add_flag | rm_flag);
            }
            else {
               change_count2_byte_acu_multi = pt2_byte_acu_multi->add_watchpoint(start, end, add_flag);
               pt2_byte_acu_multi->general_fault(start, end, add_flag);
            }
#endif
#ifdef RC_SINGLE
            range_cache_write_misses = range_cache[thread_id]->add_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
#ifdef RC_OCBM
            range_cache_ocbm_write_misses = range_cache_ocbm[thread_id]->add_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
#ifdef RC_OFFCBM
            range_cache_offcbm_write_misses = range_cache_offcbm[thread_id]->add_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
         }
         else if (rm_flag) {                                                     // For pagetables only: if (add_flag) => no need to consider rm_flag
                                                                                 //    (because they do not consider flag type)
#ifdef PAGE_TABLE
            change_count = page_table_wp[thread_id]->rm_watchpoint(start, end);       // set page_table
#endif
#ifdef PAGE_TABLE_MULTI
            change_count_multi = page_table_multi->rm_watchpoint(start, end);
#endif
#ifdef PAGE_TABLE2_SINGLE
            if (change_count)
               change_count2 = page_table2_wp[thread_id]->rm_watchpoint(start, end);     // set page_table
#endif
#ifdef PAGE_TABLE2_MULTI
            if (change_count_multi)
               change_count2_multi = page_table2_multi->rm_watchpoint(start, end);    // set page_table
#endif
#ifdef PT2_BYTE_ACU_SINGLE
            change_count2_byte_acu = pt2_byte_acu[thread_id]->rm_watchpoint(start, end, rm_flag);
            pt2_byte_acu[thread_id]->general_fault(start, end, rm_flag);
#endif
#ifdef PT2_BYTE_ACU_MULTI
            change_count2_byte_acu_multi = pt2_byte_acu_multi->rm_watchpoint(start, end, rm_flag);
            pt2_byte_acu_multi->general_fault(start, end, rm_flag);
#endif
#ifdef RC_SINGLE
            range_cache_write_misses = range_cache[thread_id]->rm_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
#ifdef RC_OCBM
            range_cache_ocbm_write_misses = range_cache_ocbm[thread_id]->rm_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
#ifdef RC_OFFCBM
            range_cache_offcbm_write_misses = range_cache_offcbm[thread_id]->rm_watchpoint(start, end, (target_flags.find("x")!=string::npos));
#endif
         }
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
         long long temp = statistics_iter->second.write_size;
         statistics_iter->second.write_size += ((long long)end-(long long)start+1);
         // If we roll over with a long long, we're in trouble.
         assert(statistics_iter->second.write_size > temp);
         statistics_iter->second.sum_size += oracle_wp_iter->second->get_size();
#ifdef PAGE_TABLE_SINGLE
         statistics_iter->second.change_count += change_count;
#endif
#ifdef PAGE_TABLE_MULTI
         statistics_iter->second.change_count_multi += change_count_multi;
#endif
#ifdef PAGE_TABLE2_SINGLE
         statistics_iter->second.change_count2 += change_count2;
#endif
#ifdef PAGE_TABLE2_MULTI
         statistics_iter->second.change_count2_multi += change_count2_multi;
#endif
#ifdef PT2_BYTE_ACU_SINGLE
         statistics_iter->second.pt2_byte_acu_changes += change_count2_byte_acu;
#endif
#ifdef PT2_BYTE_ACU_MULTI
         statistics_iter->second.pt2_byte_acu_multi_changes += change_count2_byte_acu_multi;
#endif
#ifdef MEM_TRACKER
         statistics_iter->second.mem_tracker_write_blocks += ((end-start)/16+1);
         if (mem_tracker_write_mises) {
            statistics_iter->second.mem_tracker_mem_accesses += mem_tracker_write_mises;
            statistics_iter->second.mem_tracker_write_miss++;
         }
#endif
#ifdef RC_SINGLE
         if (range_cache_write_misses) {
            statistics_iter->second.rc_write_miss++;
            statistics_iter->second.rc_backing_store_accesses += range_cache_write_misses;
         }
         else
            statistics_iter->second.rc_write_hits++;
#endif
#ifdef RC_OCBM
         if (range_cache_ocbm_write_misses) {
            statistics_iter->second.rc_ocbm_write_miss++;
            statistics_iter->second.rc_ocbm_backing_store_accesses += range_cache_ocbm_write_misses;
         }
         else
            statistics_iter->second.rc_ocbm_write_hits++;
#endif
#ifdef RC_OFFCBM
         if (range_cache_offcbm_write_misses) {
            statistics_iter->second.rc_offcbm_write_miss++;
            statistics_iter->second.rc_offcbm_backing_store_accesses += range_cache_offcbm_write_misses;
         }
         else
            statistics_iter->second.rc_offcbm_write_hits++;
#endif
      }
      return 0;                                                               // normal set: return 0
   }
   return -1;                                                                 // abnormal set: thread_id not found, return -1
}

//set  11   (r/w)
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('d', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "11", ignore_statistics);
}

//set  10
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('b', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "10", ignore_statistics);
}

//set  01
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('c', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "01", ignore_statistics);
}

//set  00
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_watch(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('a', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "00", ignore_statistics);
}

//update 1x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('e', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "1x", ignore_statistics);
}

//update x1
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::update_set_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('f', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "x1", ignore_statistics);
}

//update 0x
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_read(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('g', thread_id, start, end);
#endif
   return general_change(start, end, thread_id, "0x", ignore_statistics);
}

//update x0
template<class ADDRESS, class FLAGS>
int WatchPoint<ADDRESS, FLAGS>::rm_write(ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics) {
#if defined(PRINT_VM_TRACE) || defined(PRINT_MEMTRACKER_TRACE)
   if(emulate_hardware)
      print_trace('h', thread_id, start, end);
#endif
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
      watchpoint_t<ADDRESS, FLAGS> temp = oracle_wp_iter->second->start_traverse();  // start traversing through thread #1
      do {
         if (temp.flags & flag_thread1) {                                           // check only for required flags in thread #1
            if (oracle_wp_iter_2->second->general_fault(temp.start_addr, 
                                                       temp.end_addr, 
                                                       flag_thread2) )              // check for faults for required flags in thread #2
               return true;
         }
      } while (oracle_wp_iter->second->continue_traverse(temp) );
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
      statistics_t local_stats;
      local_stats = update_active_stats(thread_id);
      return local_stats;                                // return active statistics
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
   empty.write_size=0;
   empty.sst_insertions=0;
   empty.max_size=0;
   empty.sum_size=0;
   #ifdef PAGE_TABLE_SINGLE
   empty.page_table_faults=0;
   empty.change_count=0;
   #endif
   #ifdef PAGE_TABLE_MULTI
   empty.page_table_multi_faults=0;
   empty.change_count_multi=0;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   empty.superpage_miss=0;
   empty.superpage_miss_faults=0;
   empty.superpage_faults=0;
   empty.superpage_hits=0;
   empty.highest_faults=0;
   empty.highest_hits=0;
   empty.change_count2=0;
   #endif
   #ifdef PAGE_TABLE2_MULTI
   empty.superpage_miss_multi=0;
   empty.superpage_miss_faults_multi=0;
   empty.superpage_faults_multi=0;
   empty.superpage_hits_multi=0;
   empty.highest_faults_multi=0;
   empty.highest_hits_multi=0;
   empty.change_count2_multi=0;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   empty.pt2_byte_acu_seg_reg_hits=0;
   empty.pt2_byte_acu_seg_reg_faults=0;
   empty.pt2_byte_acu_superpage_hits=0;
   empty.pt2_byte_acu_superpage_faults=0;
   empty.pt2_byte_acu_page_hits=0;
   empty.pt2_byte_acu_page_faults=0;
   empty.pt2_byte_acu_bitmap_faults=0;
   empty.pt2_byte_acu_changes=0;
   empty.pt2_byte_acu_plb_misses=0;
   empty.pt2_byte_acu_plb_wasnt_there=0;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   empty.pt2_byte_acu_multi_seg_reg_hits=0;
   empty.pt2_byte_acu_multi_seg_reg_faults=0;
   empty.pt2_byte_acu_multi_superpage_hits=0;
   empty.pt2_byte_acu_multi_superpage_faults=0;
   empty.pt2_byte_acu_multi_page_hits=0;
   empty.pt2_byte_acu_multi_page_faults=0;
   empty.pt2_byte_acu_multi_bitmap_faults=0;
   empty.pt2_byte_acu_multi_changes=0;
   empty.pt2_byte_acu_multi_plb_misses=0;
   empty.pt2_byte_acu_multi_plb_wasnt_there=0;
   #endif
   #ifdef MEM_TRACKER
   empty.mem_tracker_read_miss=0;
   empty.mem_tracker_write_miss=0;
   empty.mem_tracker_mem_accesses=0;
   empty.mem_tracker_write_blocks=0;
   #endif
   #ifdef RC_SINGLE
   empty.rc_read_hits=0;
   empty.rc_read_miss=0;
   empty.rc_write_hits=0;
   empty.rc_write_miss=0;
   empty.rc_backing_store_accesses=0;
   empty.rc_kickout_dirties=0;
   empty.rc_kickouts=0;
   empty.rc_complex_updates=0;
   #endif
   #ifdef RC_OCBM
   empty.rc_ocbm_read_hits=0;
   empty.rc_ocbm_read_miss=0;
   empty.rc_ocbm_write_hits=0;
   empty.rc_ocbm_write_miss=0;
   empty.rc_ocbm_backing_store_accesses=0;
   empty.rc_ocbm_kickout_dirties=0;
   empty.rc_ocbm_kickouts=0;
   empty.rc_ocbm_complex_updates=0;
   #endif
   #ifdef RC_OFFCBM
   empty.rc_offcbm_read_hits=0;
   empty.rc_offcbm_read_miss=0;
   empty.rc_offcbm_write_hits=0;
   empty.rc_offcbm_write_miss=0;
   empty.rc_offcbm_wlb_miss=0;
   empty.rc_offcbm_wlb_miss_size=0;
   empty.rc_offcbm_backing_store_accesses=0;
   empty.rc_offcbm_kickout_dirties=0;
   empty.rc_offcbm_kickouts=0;
   empty.rc_offcbm_complex_updates=0;
   empty.rc_offcbm_offcbm_switch=0;
   empty.rc_offcbm_range_switch=0;
   #endif
   return empty;
}

/*
 * Print statistics thread by thread
 */
template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_statistics(ostream &output, bool active) {
   statistics_t local_stats; 
   if (active) {        // if active, only print active threads' statistics only
      output << "Printing statistics for all active threads: " << endl;
      for (statistics_iter = statistics.begin();statistics_iter != statistics.end();statistics_iter++) {
         output << "Thread " << statistics_iter->first << " statistics: " << endl;
         local_stats = update_active_stats(statistics_iter->first);
         print_statistics(local_stats, output);
      }
   }
   else {               // if !active, print all threads' statistics
      output << "Printing statistics for all threads (both active and inactive): " << endl;
      statistics_iter = statistics.begin();
      statistics_inactive_iter = statistics_inactive.begin();
      while (statistics_iter != statistics.end() || statistics_inactive_iter != statistics_inactive.end()) {
         if (statistics_inactive_iter == statistics_inactive.end()) {
            output << "Active Thread " << statistics_iter->first << " statistics: " << endl;
            local_stats = update_active_stats(statistics_iter->first);
            print_statistics(local_stats, output);
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
               local_stats = update_active_stats(statistics_iter->first);
               print_statistics(local_stats, output);
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
   output << setw(45) << "total wp write size: " << to_print.write_size << endl;
   output << setw(45) << "complexity to do SST insertions: " << to_print.sst_insertions << endl;
   output << setw(45) << "max size of the whole watchpoint system: " << to_print.max_size << endl;
   output << setw(45) << "average size: " << ( (double)to_print.sum_size/(to_print.sets+to_print.updates) ) << endl;
   #ifdef PAGE_TABLE_SINGLE
   output << setw(45) << "Page table (single) faults taken: " << to_print.page_table_faults << endl;
   output << setw(45) << "Page table (single) bitmap changes: " << to_print.change_count << endl;
   #endif
   #ifdef PAGE_TABLE_MULTI
   output << setw(45) << "Page table (multi) faults taken: " << to_print.page_table_multi_faults << endl;
   output << setw(45) << "Page table (multi) bitmap changes: " << to_print.change_count_multi << endl;
   #endif
   #ifdef PAGE_TABLE2_SINGLE
   output << setw(45) << "2 level PT (single) quick hit: " << to_print.highest_hits << endl;
   output << setw(45) << "2 level PT (single) quick fault: " << to_print.highest_faults << endl;
   output << setw(45) << "2 level PT (single) superpage hit: " << to_print.superpage_hits << endl;
   output << setw(45) << "2 level PT (single) superpage fault: " << to_print.superpage_faults << endl;
   output << setw(45) << "2 level PT (single) superpage miss: " << to_print.superpage_miss << endl;
   output << setw(45) << "2 level PT (single) low level page fault: " << to_print.superpage_miss_faults << endl;
   output << setw(45) << "2 level PT (single) bit changes: " << to_print.change_count2 << endl;
   #endif
   #ifdef PAGE_TABLE2_MULTI
   output << setw(45) << "2 level PT (multi) quick hit: " << to_print.highest_hits_multi << endl;
   output << setw(45) << "2 level PT (multi) quick fault: " << to_print.highest_faults_multi << endl;
   output << setw(45) << "2 level PT (multi) superpage hit: " << to_print.superpage_hits_multi << endl;
   output << setw(45) << "2 level PT (multi) superpage fault: " << to_print.superpage_faults_multi << endl;
   output << setw(45) << "2 level PT (multi) superpage miss: " << to_print.superpage_miss_multi << endl;
   output << setw(45) << "2 level PT (multi) low level page fault: " << to_print.superpage_miss_faults_multi << endl;
   output << setw(45) << "2 level PT (multi) bit changes: " << to_print.change_count2_multi << endl;
   #endif
   #ifdef PT2_BYTE_ACU_SINGLE
   output << setw(45) << "2 level PT trie (single) quick hit: " << to_print.pt2_byte_acu_seg_reg_hits << endl;
   output << setw(45) << "2 level PT trie (single) quick fault: " << to_print.pt2_byte_acu_seg_reg_faults << endl;
   output << setw(45) << "2 level PT trie (single) superpage hit: " << to_print.pt2_byte_acu_superpage_hits << endl;
   output << setw(45) << "2 level PT trie (single) superpage fault: " << to_print.pt2_byte_acu_superpage_faults << endl;
   output << setw(45) << "2 level PT trie (single) pagetable hit: " << to_print.pt2_byte_acu_page_hits << endl;
   output << setw(45) << "2 level PT trie (single) pagetable fault: " << to_print.pt2_byte_acu_page_faults << endl;
   output << setw(45) << "2 level PT trie (single) bitmap fault: " << to_print.pt2_byte_acu_bitmap_faults << endl;
   output << setw(45) << "2 level PT trie (single) total fault: " << to_print.pt2_byte_acu_bitmap_faults+to_print.pt2_byte_acu_page_faults+to_print.pt2_byte_acu_superpage_faults+to_print.pt2_byte_acu_seg_reg_faults << endl;
   output << setw(45) << "2 level PT trie (single) bit changes: " << to_print.pt2_byte_acu_changes << endl;
   output << setw(45) << "2 level PT trie (single) data loaded to plb: " << to_print.pt2_byte_acu_plb_misses << endl;
   output << setw(45) << "2 level PT trie (single) plb misses: " << to_print.pt2_byte_acu_plb_wasnt_there << endl;
   #endif
   #ifdef PT2_BYTE_ACU_MULTI
   output << setw(45) << "2 level PT trie (multi) quick hit: " << to_print.pt2_byte_acu_multi_seg_reg_hits << endl;
   output << setw(45) << "2 level PT trie (multi) quick fault: " << to_print.pt2_byte_acu_multi_seg_reg_faults << endl;
   output << setw(45) << "2 level PT trie (multi) superpage hit: " << to_print.pt2_byte_acu_multi_superpage_hits << endl;
   output << setw(45) << "2 level PT trie (multi) superpage fault: " << to_print.pt2_byte_acu_multi_superpage_faults << endl;
   output << setw(45) << "2 level PT trie (multi) pagetable hit: " << to_print.pt2_byte_acu_multi_page_hits << endl;
   output << setw(45) << "2 level PT trie (multi) pagetable fault: " << to_print.pt2_byte_acu_multi_page_faults << endl;
   output << setw(45) << "2 level PT trie (multi) bitmap fault: " << to_print.pt2_byte_acu_multi_bitmap_faults << endl;
   output << setw(45) << "2 level PT trie (multi) total fault: " << to_print.pt2_byte_acu_multi_bitmap_faults+to_print.pt2_byte_acu_multi_page_faults+to_print.pt2_byte_acu_multi_superpage_faults+to_print.pt2_byte_acu_multi_seg_reg_faults << endl;
   output << setw(45) << "2 level PT trie (multi) bit changes: " << to_print.pt2_byte_acu_multi_changes << endl;
   output << setw(45) << "2 level PT trie (multi) data loaded to plb: " << to_print.pt2_byte_acu_multi_plb_misses << endl;
   output << setw(45) << "2 level PT trie (multi) plb misses: " << to_print.pt2_byte_acu_multi_plb_wasnt_there << endl;
   #endif
   #ifdef MEM_TRACKER
   output << setw(45) << "MemTracker read misses: " << to_print.mem_tracker_read_miss << endl;
   output << setw(45) << "MemTracker write misses: " << to_print.mem_tracker_write_miss << endl;
   output << setw(45) << "MemTracker total bitmap reads: " << to_print.mem_tracker_mem_accesses << endl;
   output << setw(45) << "MemTracker 32bit blocks modified: " << to_print.mem_tracker_write_blocks << endl;
   #endif
   #ifdef RC_SINGLE
   output << setw(45) << "Range Cache (single) read hit: " << to_print.rc_read_hits << endl;
   output << setw(45) << "Range Cache (single) read miss: " << to_print.rc_read_miss << endl;
   output << setw(45) << "Range Cache (single) write hit: " << to_print.rc_write_hits << endl;
   output << setw(45) << "Range Cache (single) write miss: " << to_print.rc_write_miss << endl;
   output << setw(45) << "Range Cache (single) backing store access: " << to_print.rc_backing_store_accesses <<  endl;
   output << setw(45) << "Range Cache (single) kickouts dirty: " << to_print.rc_kickout_dirties << endl;
   output << setw(45) << "Range Cache (single) kickouts total: " << to_print.rc_kickouts << endl;
   output << setw(45) << "Range Cache (single) complex update: " << to_print.rc_complex_updates << endl;
   #endif
   #ifdef RC_OCBM
   output << setw(45) << "Range Cache (OCBM) read hit: " << to_print.rc_ocbm_read_hits << endl;
   output << setw(45) << "Range Cache (OCBM) read miss: " << to_print.rc_ocbm_read_miss << endl;
   output << setw(45) << "Range Cache (OCBM) write hit: " << to_print.rc_ocbm_write_hits << endl;
   output << setw(45) << "Range Cache (OCBM) write miss: " << to_print.rc_ocbm_write_miss << endl;
   output << setw(45) << "Range Cache (OCBM) backing store access: " << to_print.rc_ocbm_backing_store_accesses << endl;
   output << setw(45) << "Range Cache (OCBM) kickouts dirty: " << to_print.rc_ocbm_kickout_dirties << endl;
   output << setw(45) << "Range Cache (OCBM) kickouts total: " << to_print.rc_ocbm_kickouts << endl;
   output << setw(45) << "Range Cache (OCBM) complex update: " << to_print.rc_ocbm_complex_updates << endl;
   #endif
   #ifdef RC_OFFCBM
   output << setw(45) << "Range Cache (OffCBM) read hit: " << to_print.rc_offcbm_read_hits << endl;
   output << setw(45) << "Range Cache (OffCBM) read miss: " << to_print.rc_offcbm_read_miss << endl;
   output << setw(45) << "Range Cache (OffCBM) write hit: " << to_print.rc_offcbm_write_hits << endl;
   output << setw(45) << "Range Cache (OffCBM) write miss: " << to_print.rc_offcbm_write_miss << endl;
   output << setw(45) << "Range Cache (OffCBM) wlb miss: " << to_print.rc_offcbm_wlb_miss << endl;
   output << setw(45) << "Range Cache (OffCBM) 32bit blocks modified: " << to_print.rc_offcbm_wlb_miss_size << endl;
   output << setw(45) << "Range Cache (OffCBM) backing store access: " << to_print.rc_offcbm_backing_store_accesses << endl;
   output << setw(45) << "Range Cache (OffCBM) kickouts dirty: " << to_print.rc_offcbm_kickout_dirties << endl;
   output << setw(45) << "Range Cache (OffCBM) kickouts total: " << to_print.rc_offcbm_kickouts << endl;
   output << setw(45) << "Range Cache (OffCBM) complex update: " << to_print.rc_offcbm_complex_updates << endl;
   output << setw(45) << "Range Cache (OffCBM) switches to offcbm: " << to_print.rc_offcbm_offcbm_switch << endl;
   output << setw(45) << "Range Cache (OffCBM) switches to ranges: " << to_print.rc_offcbm_range_switch << endl;
   #endif
   output << endl;
   return;
}

template<class ADDRESS, class FLAGS>
void WatchPoint<ADDRESS, FLAGS>::print_size(int32_t thread_id, ostream &output) {
   output << oracle_wp[thread_id]->get_size() << endl;
}
#endif /* WATCHPOINT_CPP_ */
