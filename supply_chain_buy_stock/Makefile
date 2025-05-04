CXX = g++
CXXFLAGS = -g -Wall -pthread

all: testrun

testrun: testrun.o hw2.o
	$(CXX) -o testrun testrun.o hw2.o -lpthread

testrun.o: hw2.h testrun.cpp
	$(CXX) $(CXXFLAGS) -c testrun.cpp

hw2.o: hw2.h hw2.cpp monitor.h
	$(CXX) $(CXXFLAGS) -c hw2.cpp

clean:
	rm -f *.o testrun
