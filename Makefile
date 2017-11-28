CXX = g++
CXXFAGS = -std=c++11 -Wall -Wextra -pedantic -O0 -g
LDFLAGS = -lsimlib
LINK.o = $(LINK.cpp)

PROGRAM = main

$(PROGRAM): $(PROGRAM).o

$(PROGRAM).o: $(PROGRAM).cpp

clean:
	rm -f *.o $(PROGRAM)
