#ifndef PAGE_TABLE1_SINGLE_CPP_
#define PAGE_TABLE1_SINGLE_CPP_

#include "page_table1_single.h"
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
PageTable1_single<ADDRESS, FLAGS>::PageTable1_single(Oracle<ADDRESS, FLAGS> *wp_ref) {
	wp = wp_ref;
	for (int i=0;i<BIT_MAP_NUMBER;i++)
		bit_map[i] = 0;
}

template<class ADDRESS, class FLAGS>
PageTable1_single<ADDRESS, FLAGS>::PageTable1_single() {
   wp = NULL;
	for (int i=0;i<BIT_MAP_NUMBER;i++)
		bit_map[i] = 0;
}

//	Desctructor
template<class ADDRESS, class FLAGS>
PageTable1_single<ADDRESS, FLAGS>::~PageTable1_single() {
}

//	print only pages being watched, used for debugging
template<class ADDRESS, class FLAGS>
void PageTable1_single<ADDRESS, FLAGS>::watch_print(ostream &output) {
	output <<"Start printing watchpoints..."<<endl;
	for (ADDRESS i=0;i<PAGE_NUMBER;i++) {
		if (bit_map[i>>3] & (1<<(i&0x7)) )
			output <<"page number "<<i<<" is being watched."<<endl;
	}
	return;
}

//	watch_fault
template<class ADDRESS, class FLAGS>
bool PageTable1_single<ADDRESS, FLAGS>::watch_fault(ADDRESS start_addr, ADDRESS end_addr) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {  //	for each page, 
		if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7)) )   //	if it is watched, 
			return true;                                          //	then return true.
	}
	return false;                                               //	else return false.
}

// version 2: count all related pages  (faster)  (do not add them up---change WatchPoint.general_change() )
//	add_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++)       //	for each page, 
		bit_map[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));          //	set the page watched. (overwrite)
	return page_number_end - page_number_start + 1;
}

//	rm_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   int changes = 0;
   //	calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   for (ADDRESS i=page_number_start;i<=page_number_end;i++) {                          //	for each page, 
	   if (!wp->watch_fault(i<<PAGE_OFFSET_LENGTH, ((i+1)<<PAGE_OFFSET_LENGTH)-1 ) ) {  //	if it should not throw a fault
		   bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                           //	set the page unwatched (overwrite)
		   changes++;
	   }
   }
   return changes;
}

/*
// version 1: count the switchings of the bit_map
//	add_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::add_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   int changes = 0;
	//	calculating the starting V.P.N. and the ending V.P.N.
	ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
	ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
	for (ADDRESS i=page_number_start;i<=page_number_end;i++) {     //	for each page, 
	   if (!(bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) ) { // if needs to be changed
	      changes++;                                               // change++
		   bit_map[i>>BIT_MAP_OFFSET_LENGTH] |= (1<<(i&0x7));       //	set the page watched.
		}
	}
	return changes;
}

//	rm_watchpoint
template<class ADDRESS, class FLAGS>
int PageTable1_single<ADDRESS, FLAGS>::rm_watchpoint(ADDRESS start_addr, ADDRESS end_addr) {
   int changes = 0;
   //	calculating the starting V.P.N. and the ending V.P.N.
   ADDRESS page_number_start = (start_addr>>PAGE_OFFSET_LENGTH);
   ADDRESS page_number_end = (end_addr>>PAGE_OFFSET_LENGTH);
   for (ADDRESS i=page_number_start;i<=page_number_end;i++) {                          //	for each page, 
	   if (!wp->watch_fault(i<<PAGE_OFFSET_LENGTH, ((i+1)<<PAGE_OFFSET_LENGTH)-1 ) ) {  //	if it should not throw a fault
	      if (bit_map[i>>BIT_MAP_OFFSET_LENGTH] & (1<<(i&0x7))) {                       // if needs to be changed
	         changes++;                                                                 // change++
		      bit_map[i>>BIT_MAP_OFFSET_LENGTH] &= ~(1<<(i&0x7));                        //	set the page unwatched
		   }
	   }
   }
   return changes;
}
*/

#endif
