/*
 *	watchpoint.h
 *
 *	Created on: May 4, 2011
 *		Author: luoyixin
 */

#ifndef WATCHPOINT_H_
#define WATCHPOINT_H_

#include <stdint.h>     //contains int32_t
#include <map>
#include "oracle_wp.h"

/*
 *	Structure that contains all hardware emulation statistics for one thread
 */
struct statistics_t {
	long long checks;          //	total number of checks on faults
	long long oracle_faults;   //	total number of times that the Oracle get faults
	long long sets;            //	total number of times the API sets a watchpoint
	long long updates;         //	total number of times the API updates a watchpoint
};

/*
 *	operator that is used for a += b
 */
inline statistics_t& operator +=(statistics_t &a, const statistics_t &b);

/*
 *	operator that is used for c = a + b
 *		adding all statistics of a and b and then return the result
 */
inline statistics_t operator +(const statistics_t &a, const statistics_t &b);

/*
 *	WatchPoint Library API
 */
template<class ADDRESS, class FLAGS>
class WatchPoint {
public:
	WatchPoint();  //	Constructor
	~WatchPoint(); //	Destructor
	
	//Threading Calls
	int start_thread(int32_t thread_id);   // returns 0 on success, -1 on failure
                                          //    (can't have multiple active threads holding the same thread_id)
	int end_thread(int32_t thread_id);     // turns an active thread into inactive state
	void print_threads();                  // prints all active thread_id's
	
	//Reset Functions
	void reset();                          // resets all threads, whether active or inactive
	void reset(int32_t thread_id);         // resets thread #thread_id if found, whether active or inactive
	
	//Checking Watchpoints
	bool watch_fault     (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //	check for r/w faults
	bool read_fault      (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //	only check r faults
	bool write_fault     (ADDRESS start, ADDRESS end, int32_t thread_id, bool ignore_statistics=false);   //	only check w faults
	void print_watchpoints();
	
	//Changing Watchpoints
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
	 *	Check every flag_thread1 watchpoints in thread_id1 and
	 *	      every flag_thread2 watchpoints in thread_id2
	 *	to see if any overlap exists.
	 */
	bool general_compare (int32_t thread_id1, int32_t thread_id2, FLAGS flag_thread1, FLAGS flag_thread2);
	bool compare_rr      (int32_t thread_id1, int32_t thread_id2);
	bool compare_rw      (int32_t thread_id1, int32_t thread_id2);
	bool compare_wr      (int32_t thread_id1, int32_t thread_id2);
	bool compare_ww      (int32_t thread_id1, int32_t thread_id2);
	
	//Hardware Emulation Statistic Functions
	statistics_t get_statistics   (int32_t thread_id);
	int          set_statistics   (int32_t thread_id, statistics_t input);
	statistics_t clear_statistics ();
	void         print_statistics (bool active = false);
	void         print_statistics (const statistics_t& to_print);

private:
	//Data Structures (mainly maps from thread_id's to the thread's data)
	map<int32_t, Oracle<ADDRESS, FLAGS> >                    oracle_wp;           //	for storing byte accurate emulation class
	map<int32_t, statistics_t>                               statistics;          //	for storing all active threads and statistics
	map<int32_t, statistics_t>                               statistics_inactive; //	for storing all inactive threads and statistics (no overlaps)
	
	//Useful iterators
	typename map<int32_t, Oracle<ADDRESS, FLAGS> >::iterator oracle_wp_iter;
	typename map<int32_t, Oracle<ADDRESS, FLAGS> >::iterator oracle_wp_iter_2;
	map<int32_t, statistics_t>::iterator                     statistics_iter;
	map<int32_t, statistics_t>::iterator                     statistics_inactive_iter;
};

#include "watchpoint.cpp"
#endif /* WATCHPOINT_H_ */
