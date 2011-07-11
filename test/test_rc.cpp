#include "../watchpoint_system/range_cache.h"

int main(int argc, char *argv[]) {
   Oracle<unsigned int, unsigned int> oracle_wp;
	RangeCache<unsigned int, unsigned int> rc_test(&oracle_wp);
	string instr;
	unsigned int start, end;
	string flag;
	int rc_miss;

	ostream *output;
	if (argc==1) {
		output = &cout;
	}
	else {
		output = new ofstream(argv[1]);
	}
	cout <<"Please enter the operation you want to perform on range cache: <operation> <start> <end> <flag(rw)> "<<endl;
	cout <<"Instructions list: "<<endl;
	cout <<"1. read:  eg.: > read  51 100 rw " <<endl;
	cout <<"2. write: eg.: > write 51 100 1x " <<endl;
	cout <<"3. stats: print statistics in range cache "<<endl;
	cout <<"4. quit: input \"quit\" to quit this test program. "<<endl;
	cout <<"> ";
	cin >>instr;
	while (instr != "quit") {
	   if (instr == "read") {
	      cin >>start >>end >>flag;
	      // whatever flag it is
			rc_miss = rc_test.watch_fault(start, end);
			cout <<"Printing range cache entries......"<<endl;
		   rc_test.watch_print(*output);
		   if (rc_miss)
			   cout <<"Number of backing store accesses: "<<rc_miss<<endl;
	   }
	   else if (instr == "write") {
	      cin >>start >>end >>flag;
		   if (flag == "11") {
			   oracle_wp.add_watchpoint(start, end, WA_READ|WA_WRITE);
			   rc_miss = rc_test.wp_operation(start, end, false);
			}
		   else if (flag == "10") {
			   oracle_wp.add_watchpoint(start, end, WA_READ);
			   oracle_wp.rm_watchpoint(start, end, WA_WRITE);
			   rc_miss = rc_test.wp_operation(start, end, false);
			}
		   else if (flag == "01") {
			   oracle_wp.add_watchpoint(start, end, WA_WRITE);
			   oracle_wp.rm_watchpoint(start, end, WA_READ);
			   rc_miss = rc_test.wp_operation(start, end, false);
			}
		   else if (flag == "00") {
			   oracle_wp.rm_watchpoint(start, end, WA_READ|WA_WRITE);
			   rc_miss = rc_test.wp_operation(start, end, false);
			}
		   else if (flag == "1x") {
			   oracle_wp.add_watchpoint(start, end, WA_READ);
			   rc_miss = rc_test.wp_operation(start, end, true);
			}
		   else if (flag == "x1") {
			   oracle_wp.add_watchpoint(start, end, WA_WRITE);
			   rc_miss = rc_test.wp_operation(start, end, true);
			}
		   else if (flag == "0x") {
			   oracle_wp.rm_watchpoint(start, end, WA_READ);
			   rc_miss = rc_test.wp_operation(start, end, true);
			}
		   else if (flag == "x0") {
			   oracle_wp.rm_watchpoint(start, end, WA_WRITE);
			   rc_miss = rc_test.wp_operation(start, end, true);
			}
		   else {
			   rc_miss = -1;
			   cout <<"Invalid flag! Flag format: 11 10 01 00 1x x1 0x x0."<<endl;
		   }
		   if (rc_miss != -1) {
			   cout <<"Printing range cache entries......"<<endl;
		      rc_test.watch_print(*output);
		      if (rc_miss)
			      cout <<"Number of backing store accesses: "<<rc_miss<<endl;
		   }
	   }
	   else if (instr == "stats") {
	      cout <<"Printing internal statistics: "<<endl;
	      cout <<setw(18)<<"kickouts dirty: "<<rc_test.kickout_dirty<<endl;
	      cout <<setw(18)<<"kickouts total: "<<rc_test.kickout<<endl;
	      cout <<setw(18)<<"complex updates: "<<rc_test.complex_updates<<endl;
	   }
	   else {
	      cout <<endl<<"invalid instruction: "<<instr<<endl;
	      cout <<"input \"quit\" to quit this test program. "<<endl;
	   }
	   cout <<"> ";
	   cin >>instr;
	}
	return 0;
}
