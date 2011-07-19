CXX=g++
CFLAGS=-Wall -Werror -O3
EXECS=test_add test_rm test_cache_algo test_print test_pt test_wp test_rc pintools	#add executes here
OBJS=$(WP_FOLDER)$(ORACLE).o $(WP_FOLDER)$(PT).o $(WP_FOLDER)$(PT_MULTI).o $(WP_FOLDER)$(PT2).o $(WP_FOLDER)$(PT2_BYTE_ACU).o $(WP_FOLDER)$(CACHE_ALGO).o $(WP_FOLDER)$(RC).o $(WP_FOLDER)$(WATCHPOINT).o 	#add objects here

WP_FOLDER=watchpoint_system/
WP_DATA=wp_data_struct
ORACLE=oracle_wp
PT=page_table1_single
PT_MULTI=page_table1_multi
PT2=page_table2_single
PT2_BYTE_ACU=pt2_byte_acu_single
CACHE_ALGO=cache_algo
RC=range_cache
#add objects here
WATCHPOINT=watchpoint
PINTOOLS=pintools
.PHONY : pintools clean

ALL_TEST=$(TEST_ADD) $(TEST_RM) $(TEST_CACHE_ALGO) $(TEST_PRINT) $(TEST_PT) $(TEST_WP) $(TEST_RC)  #add tests here
TEST_ADD=test/test_add
TEST_RM=test/test_rm
TEST_CACHE_ALGO=test/test_cache_algo
TEST_PRINT=test/test_print
TEST_PT=test/test_pt
TEST_WP=test/test_wp
TEST_RC=test/test_rc

all: $(ALL_TEST) $(PINTOOLS)

$(TEST_ADD): $(OBJS) $(TEST_ADD).cpp
	$(CXX) $(CFLAGS) $(TEST_ADD).cpp -o test_add

$(TEST_RM): $(OBJS) $(TEST_RM).cpp
	$(CXX) $(CFLAGS) $(TEST_RM).cpp -o test_rm

$(TEST_CACHE_ALGO): $(OBJS) $(TEST_CACHE_ALGO).cpp
	$(CXX) $(CFLAGS) $(TEST_CACHE_ALGO).cpp -o test_cache_algo

$(TEST_PRINT): $(OBJS) $(TEST_PRINT).cpp
	$(CXX) $(CFLAGS) $(TEST_PRINT).cpp -o test_print

$(TEST_PT): $(OBJS) $(TEST_PT).cpp
	$(CXX) $(CFLAGS) $(TEST_PT).cpp -o test_pt

$(TEST_WP): $(OBJS) $(TEST_WP).cpp
	$(CXX) $(CFLAGS) $(TEST_WP).cpp -o test_wp

$(TEST_RC): $(OBJS) $(TEST_RC).cpp
	$(CXX) $(CFLAGS) $(TEST_RC).cpp -o test_rc

#add tests here

$(ORACLE).o: $(ORACLE).cpp $(ORACLE).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(ORACLE).cpp

$(PT).o: $(PT).cpp $(PT).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(PT).cpp

$(PT_MULTI).o: $(PT_MULTI).cpp $(PT_MULTI).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(PT_MULTI).cpp

$(PT2).o: $(PT2).cpp $(PT2).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(PT2).cpp

$(PT2_BYTE_ACU).o: $(PT2_BYTE_ACU).cpp $(PT2_BYTE_ACU).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(PT2_BYTE_ACU).cpp

$(CACHE_ALGO).o: $(CACHE_ALGO).cpp $(CACHE_ALGO).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(CACHE_ALGO).cpp

$(RC).o: $(RC).cpp $(RC).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(RC).cpp

$(WATCHPOINT).o: $(WATCHPOINT).cpp $(WATCHPOINT).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(WATCHPOINT).cpp

#add objects here

$(PINTOOLS):
	cd pintools; $(MAKE) clean; $(MAKE)

clean:
	rm $(EXECS) $(WP_FOLDER)*.o; cd pintools; $(MAKE) clean

