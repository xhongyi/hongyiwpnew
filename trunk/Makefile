CXX=g++
CFLAGS = -g -Wall -Werror
EXECS=test_add test_rm test_cache_algo test_print test_pt	#add executes here
OBJS=$(WP_FOLDER)$(ORACLE).o $(WP_FOLDER)$(PT).o $(WP_FOLDER)$(CACHE_ALGO).o #$(WP_FOLDER)$(RC).o	#add objects here

WP_FOLDER=watchpoint_system/
WP_DATA=wp_data_struct
ORACLE=oracle_wp
PT=page_table_wp
CACHE_ALGO=cache_algo
#RC=range_cache

ALL_TEST=$(TEST_ADD) $(TEST_RM) $(TEST_CACHE_ALGO) $(TEST_PRINT) $(TEST_PT) 	#add tests here
TEST_ADD=test/test_add
TEST_RM=test/test_rm
TEST_CACHE_ALGO=test/test_cache_algo
TEST_PRINT=test/test_print
TEST_PT=test/test_pt

all: $(ALL_TEST)

$(TEST_ADD): $(OBJS) $(TEST_ADD).cpp
	$(CXX) $(CFLAGS) $(TEST_ADD).cpp $(OBJS) -o test_add

$(TEST_RM): $(OBJS) $(TEST_RM).cpp
	$(CXX) $(CFLAGS) $(OBJS) $(TEST_RM).cpp -o test_rm

$(TEST_CACHE_ALGO): $(OBJS) $(TEST_CACHE_ALGO).cpp
	$(CXX) $(CFLAGS) $(OBJS) $(TEST_CACHE_ALGO).cpp -o test_cache_algo

$(TEST_PRINT): $(OBJS) $(TEST_PRINT).cpp
	$(CXX) $(CFLAGS) $(OBJS) $(TEST_PRINT).cpp -o test_print

$(TEST_PT): $(OBJS) $(TEST_PT).cpp
	$(CXX) $(CFLAGS) $(OBJS) $(TEST_PT).cpp -o test_pt

$(ORACLE).o: $(ORACLE).cpp $(ORACLE).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(ORACLE).cpp

$(PT).o: $(PT).cpp $(PT).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(PT).cpp

$(CACHE_ALGO).o: $(CACHE_ALGO).cpp $(CACHE_ALGO).h
	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(CACHE_ALGO).cpp

#$(RC).o: $(RC).cpp $(RC).h
#	cd $(WP_FOLDER) & $(CXX) $(CFLAGS) -c $(RC).cpp

clean:
	rm $(EXECS) $(WP_FOLDER)*.o

