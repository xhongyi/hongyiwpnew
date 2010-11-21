CXX=g++
CFLAGS = -g -Wall -Werror
EXECS=test
MAIN=test_add
MAIN_INC=auto_wp.h
OBJS=$(MAIN).o

all: $(EXECS)

$(EXECS): $(OBJS)
	$(CXX) $(CFLAGS) -o $(EXECS) $(OBJS)

$(MAIN).o : $(MAIN).cpp $(MAIN_INC)
	$(CXX) $(CFLAGS) -c $(MAIN).cpp

clean:
	rm $(EXECS) *.o
