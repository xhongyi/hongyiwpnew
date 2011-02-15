CXX=g++
CFLAGS = -g -Wall -Werror
EXECS=test
MAIN=test_cache_algo
MAIN_INC=cache_algo.h
OBJS=$(MAIN).o

all:
	g++ -Wall -Werror test_cache_algo.cpp -o a.out
#$(EXECS)

#$(EXECS): $(OBJS)
#	$(CXX) $(CFLAGS) -o $(EXECS) $(OBJS)

#$(MAIN).o : $(MAIN).cpp $(MAIN_INC)
#	$(CXX) $(CFLAGS) -c $(MAIN).cpp

#clean:
#	rm $(EXECS) *.o
