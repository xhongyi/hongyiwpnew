#ifndef IWATCHER_CPP_
#define IWATCHER_CPP_

#include "iwatcher.h"

template<class ADDRESS, class FLAGS>
IWatcher<ADDRESS, FLAGS>::IWatcher(ostream &output) : trace_output(output) {
   assert(L2_BLOCK_OFFSET>=L1_BLOCK_OFFSET);
   l2_data = new L2Cache<ADDRESS>;
   vm_sys = new PageTable1_single<ADDRESS, FLAGS>;
   vm_changes = 0;
   victim_kickouts = 0;
   large_sets = 0;
}

template<class ADDRESS, class FLAGS>
IWatcher<ADDRESS, FLAGS>::~IWatcher() {
   delete l2_data;
   delete vm_sys;
}

template<class ADDRESS, class FLAGS>
void IWatcher<ADDRESS, FLAGS>::start_thread(int32_t thread_id, Oracle<ADDRESS, FLAGS> *wp_ref) {
   wp[thread_id] = wp_ref;
   l1_data[thread_id] = new L1Cache<ADDRESS>;
}

template<class ADDRESS, class FLAGS>
void IWatcher<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   wp.erase(thread_id);
   delete l1_data.find(thread_id)->second;
   l1_data.erase(thread_id);
}

template<class ADDRESS, class FLAGS>
int IWatcher<ADDRESS, FLAGS>::general_fault(int32_t thread_id, ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   int cache_miss = 0;
   ADDRESS start_idx = (start_addr>>L1_BLOCK_OFFSET);
   ADDRESS end_idx   = (end_addr  >>L1_BLOCK_OFFSET);
   for (ADDRESS i=start_idx;i<=end_idx;i++) {
   // check in l1
      if (l1_data[thread_id]->check_and_update(i)) {          // if hit in l1
         break;
      }
   // check in l2 and victim cache
      else if (l2_data->check_and_update(thread_id, (i>>(L2_BLOCK_OFFSET-L1_BLOCK_OFFSET)))) {
                                                            // if hit in l2
         break;
      }
      else if (l2_data->victim_check_and_rm(thread_id, (i>>(L2_BLOCK_OFFSET-L1_BLOCK_OFFSET)))) {
                                                            // if hit and removed in victim cache
         break;
      }
   // check in vm
      else {
         cache_miss++;
         while (l2_data->victim_overflow()) {
            victim_kickouts++;
            l2_data->victim_kickout();
            victim_kickout_handler();
         }
         if (vm_sys->general_fault((i<<L1_BLOCK_OFFSET), (i<<L1_BLOCK_OFFSET), target_flags)) {
                                                            // if unavailable
            load_page(thread_id, (i>>(PAGE_OFFSET_LENGTH-L1_BLOCK_OFFSET)));               // load all watchpoints in this page
            vm_sys->rm_watchpoint((i<<L1_BLOCK_OFFSET), (i<<L1_BLOCK_OFFSET), WA_READ|WA_WRITE); // and mark the page as available
            l1_data[thread_id]->check_and_update(i);
            l2_data->check_and_update(thread_id, i>>(L2_BLOCK_OFFSET-L1_BLOCK_OFFSET));
            while (l2_data->victim_overflow()) {
               victim_kickouts++;
               l2_data->victim_kickout();
               victim_kickout_handler();
            }
         }
      }
   }
   assert(l2_data->writeback_buffer.tag == (ADDRESS)-1);
   assert(!l2_data->victim_overflow());
   return cache_miss;
}

template<class ADDRESS, class FLAGS>
void IWatcher<ADDRESS, FLAGS>::victim_kickout_handler() {
   if (l2_data->writeback_buffer.tag != (ADDRESS)-1) {
      L2CacheEntry<ADDRESS> temp = l2_data->writeback_buffer;
      l2_data->writeback_buffer.tag = (ADDRESS)-1;          // set as invalid
      // mark the page in vm as unavailable if necessary
      if (wp[temp.thread_id]->watch_fault((temp.tag<<L2_BLOCK_OFFSET), ((temp.tag+1)<<L2_BLOCK_OFFSET)-1))
         vm_changes += vm_sys->add_watchpoint((temp.tag<<L2_BLOCK_OFFSET), (temp.tag<<L2_BLOCK_OFFSET), WA_READ|WA_WRITE);
   }
}

template<class ADDRESS, class FLAGS>
int IWatcher<ADDRESS, FLAGS>::add_watchpoint(int32_t thread_id, ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   ADDRESS start_idx = (start_addr>>L1_BLOCK_OFFSET);
   ADDRESS end_idx   = (end_addr  >>L1_BLOCK_OFFSET);
   // update lru if exist
   for (ADDRESS i=start_idx;i<=end_idx;i++) {
      l1_data[thread_id]->update_if_exist(i);
      l2_data->update_if_exist(thread_id, i);
   }
   // mark the page as unavailble
   int changes = vm_sys->add_watchpoint(start_addr, end_addr, target_flags);
   vm_changes += changes;
   assert(!l2_data->victim_overflow());
   return changes;
}

template<class ADDRESS, class FLAGS>
int IWatcher<ADDRESS, FLAGS>::rm_watchpoint(int32_t thread_id, ADDRESS start_addr, ADDRESS end_addr, FLAGS target_flags) {
   ADDRESS start_idx = (start_addr>>L1_BLOCK_OFFSET);
   ADDRESS end_idx   = (end_addr  >>L1_BLOCK_OFFSET);
   // update lru if exist
   for (ADDRESS i=start_idx;i<=end_idx;i++) {
      l1_data[thread_id]->update_if_exist(i);
   }
   start_idx = (start_addr>>L2_BLOCK_OFFSET);
   end_idx   = (end_addr  >>L2_BLOCK_OFFSET);
   for (ADDRESS i=start_idx;i<=end_idx;i++) {
      l2_data->update_if_exist(thread_id, i);
   }
   int changes = 0;
   // if large set, do not touch virtual memory
   if (end_addr - start_addr > (1<<14))
      large_sets++;
   else {
      // mark the page as availble if necessary
      ADDRESS start_page = (start_addr>>PAGE_OFFSET_LENGTH);
      ADDRESS end_page   = (end_addr  >>PAGE_OFFSET_LENGTH);
      for (ADDRESS i=start_page;i<=end_page;i++) {
         if (   vm_sys->general_fault((i<<PAGE_OFFSET_LENGTH), (i<<PAGE_OFFSET_LENGTH), WA_READ|WA_WRITE)
             && !wp[thread_id]->watch_fault((i<<PAGE_OFFSET_LENGTH), ((i+1)<<PAGE_OFFSET_LENGTH)-1)     ) {
            vm_sys->add_watchpoint((i<<PAGE_OFFSET_LENGTH), (i<<PAGE_OFFSET_LENGTH), WA_READ|WA_WRITE);
            changes++;
         }
      }
      vm_changes += changes;
   }
   return changes;
}

template<class ADDRESS, class FLAGS>
void IWatcher<ADDRESS, FLAGS>::load_page(int32_t thread_id, ADDRESS page_number) {
   ADDRESS page_start = (page_number<<PAGE_OFFSET_LENGTH);
   ADDRESS page_end = ((page_number+1)<<PAGE_OFFSET_LENGTH)-1;
   while (page_end>=page_start) {
      typename deque<watchpoint_t<ADDRESS, FLAGS> >::iterator 
         load_iter = wp[thread_id]->search_address(page_start);
      if (load_iter->flags) {    // load segments with watchpoints into l1 and l2
         ADDRESS start_addr = max(page_start, load_iter->start_addr);
         ADDRESS end_addr = min(page_end, load_iter->end_addr);
         ADDRESS start_idx = (start_addr>>L1_BLOCK_OFFSET);
         ADDRESS end_idx   = (end_addr  >>L1_BLOCK_OFFSET);
         for (ADDRESS i=start_idx;i<=end_idx;i++) {
            print_trace('k', thread_id, (i<<L1_BLOCK_OFFSET), ((i+1)<<L1_BLOCK_OFFSET)-1);
            l1_data[thread_id]->check_and_update(i);
         }
         start_idx = (start_addr>>L2_BLOCK_OFFSET);
         end_idx   = (end_addr  >>L2_BLOCK_OFFSET);
         for (ADDRESS i=start_idx;i<=end_idx;i++) {
            l2_data->check_and_update(thread_id, i);
         }
      }
      if (load_iter->end_addr >= page_end)
         break;
      page_start = load_iter->end_addr+1;
   }
}

template<class ADDRESS, class FLAGS>
void IWatcher<ADDRESS, FLAGS>::print_trace(char command, int thread_id, unsigned int starter, unsigned int ender)
{
#ifdef PRINT_IWATCHER_TRACE
   if(total_print_number < MAX_PRINT_NUM) {
       total_print_number++;
       trace_output << command << setw(5) << thread_id << setw(8) << starter << setw(8) << ender << endl;
   }
#endif
}

#endif /*IWATCHER_CPP_*/
