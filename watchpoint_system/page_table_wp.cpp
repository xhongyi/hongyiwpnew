#ifndef PAGE_TABLE_WP_CPP_
#define PAGE_TABLE_WP_CPP_

#include "page_table_wp.h"
#include <ostream>
#include <fstream>

using namespace std;

/*
 *	Constructor
 *	Initialize the wp pointer 
 *	(wp should always be constructed and modified before the page table version)
 *	And also initialize all pages to unwatched.
 */
template<class ADDRESS, class FLAGS>
WatchPoint_PT<ADDRESS, FLAGS>::WatchPoint_PT() {
   log_max_threads = 0;
   avail_bit_map_num.push(0);
   bit_map = new unsigned int[BIT_MAP_NUMBER];
	for (int i=0;i<BIT_MAP_NUMBER;i++)
		bit_map[i] = 0;
}

//	Desctructor
template<class ADDRESS, class FLAGS>
WatchPoint_PT<ADDRESS, FLAGS>::~WatchPoint_PT() {
   if (bit_map)
      delete [] bit_map;
   bitmap = NULL;
}

template<class ADDRESS, class FLAGS>
void WatchPoint_PT<ADDRESS, FLAGS>::start_thread(int32_t thread_id, Oracle<ADDRESS, FLAGS>* oracle_ptr_in) {
   oracle_wp_ptr[thread_id] = oracle_ptr_in;
   if (avail_bit_map_num.empty()) {
      for (int i=(1<<log_max_threads);i<(2<<log_max_threads);i++) {
         avail_bit_map_num.push(i);
      }
      bit_map_num[thread_id] = avail_bit_map_num.front();
      avail_bit_map_num.pop();
      /*
       * managing bit_map
       */
      unsigned int *new_bit_map = new unsigned int[BIT_MAP_NUMBER<<(log_max_threads+1)];
      for (int i=0;i<BIT_MAP_NUMBER<<(log_max_threads+1);i++) {
         new_bit_map[i] = 0;
      }
      for (int i=0;i<PAGE_NUMBER;i++) {
         new_bit_map[i>>(BIT_MAP_OFFSET_LENGTH-log_max_threads-1)] |= 
      }
      log_max_threads++;
   }
   else {
      bit_map_num[thread_id] = avail_bit_map_num.front();
      avail_bit_map_num.pop();
   }
}

template<class ADDRESS, class FLAGS>
void WatchPoint_PT<ADDRESS, FLAGS>::end_thread(int32_t thread_id) {
   
}

//	print only pages being watched, used for debugging
template<class ADDRESS, class FLAGS>
void WatchPoint_PT<ADDRESS, FLAGS>::watch_print(ostream &output) {
	output <<"Start printing watchpoints..."<<endl;
	for (ADDRESS i=0;i<PAGE_NUMBER;i++) {
		if (bit_map[i>>3] & (1<<(i&0x7)) )
			output <<"page number "<<i<<" is being watched."<<endl;
	}
	return;
}

//	watch_fault
template<class ADDRESS, class FLAGS>
bool WatchPoint_PT<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {  //	for each page, 
		if (get_bit(i))   //	if it is watched, 
			return true;                                          //	then return true.
	}
	return false;                                               //	else return false.
}

//	add_watchpoint
template<class ADDRESS, class FLAGS>
int WatchPoint_PT<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr, int32_t thread_id, FLAGS target_flags) {
	int num_changes = 0;                                              //	initializing the count
	if (target_flags) {                                               //	if the flag is not none, continue
		//	calculating the starting V.P.N. and the ending V.P.N.
		ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
		ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
		for (ADDRESS i=page_number_start;i<=page_number_end;i++) {     //	for each page, 
			if (get_bit(i) == 0)	//	if it is not watched, 
				num_changes++;                                           //	count++
			set_bit(thread_id, i);
		}
	}
	return num_changes;                                               //	return the count.
}

//	rm_watchpoint
template<class ADDRESS, class FLAGS>
int WatchPoint_PT<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr, int32_t thread_id) {
	int num_changes = 0;                                                       //	initializing the count
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {                          //	for each page, 
		if (!oracle_wp_ptr[thread_id]->watch_fault(i<<PAGE_OFFSET_LENGTH, ((i+1)<<PAGE_OFFSET_LENGTH)-1 ) ) {  //	if it should not throw a fault
			if (get_bit(i))                        //	if it is watched
				num_changes++;                                                             //	count++
			reset_bit(thread_id, i);
			if (get_bit(i))                        //	if it is watched
				num_changes--;                                                             //	count--
		}
	}
	return num_changes;                                                                 //	return the count.
}

template<class ADDRESS, class FLAGS
void WatchPoint_PT<ADDRESS, FLAGS>::set_bit(int32_t thread_id, ADDRESS page_number) {
   bit_map[ page_number>>(BIT_MAP_OFFSET_LENGTH-log_max_threads) ] |= 
			   ( (1<<(bit_map_num[thread_id]))/*content*/<<
			   ((page_number & ((1<<(BIT_MAP_OFFSET_LENGTH-log_max_threads))-1))<<log_max_threads));          //	set the page watched.
}

template<class ADDRESS, class FLAGS
void WatchPoint_PT<ADDRESS, FLAGS>::reset_bit(int32_t thread_id, ADDRESS page_number) {
   bit_map[ page_number>>(BIT_MAP_OFFSET_LENGTH-log_max_threads) ] &= 
			  ~( (1<<(bit_map_num[thread_id]))/*content*/<<
			   ((page_number & ((1<<(BIT_MAP_OFFSET_LENGTH-log_max_threads))-1))<<log_max_threads));                           //	set the page unwatched
}

template<class ADDRESS, class FLAGS
unsigned int WatchPoint_PT<ADDRESS, FLAGS>::get_bit(ADDRESS page_number) {
   return ((bit_map[i>>(BIT_MAP_OFFSET_LENGTH-log_max_threads)] >> 
			       (page_number & ((1<<(BIT_MAP_OFFSET_LENGTH-log_max_threads))-1)<<log_max_threads)/*displacement*/) &
			       ( ((1<<(1<<log_max_threads))-1)/*content*/ ) );
}

template<class ADDRESS, class FLAGS
unsigned int WatchPoint_PT<ADDRESS, FLAGS>::get_bit(ADDRESS page_number, int32_t thread_id) {
   return ((bit_map[i>>(BIT_MAP_OFFSET_LENGTH-log_max_threads)] >> 
			       (page_number & ((1<<(BIT_MAP_OFFSET_LENGTH-log_max_threads))-1)<<log_max_threads)/*displacement*/) &
			       (1<<(bit_map_num[thread_id])) );            //shift or not shift???
}

#endif
