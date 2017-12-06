CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -O0 -g
LDFLAGS = -lsimlib -lm
LINK.o = $(LINK.cpp)

PROGRAM = main
EXPERIMENTS = exp1 exp2 exp3 exp4 exp5

default: $(PROGRAM) $(EXPERIMENTS)

$(PROGRAM): $(PROGRAM).o

$(PROGRAM).o: $(PROGRAM).cpp

exp1: $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROGRAM).cpp -o exp1 -DEXPERIMENT_1

exp2: $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROGRAM).cpp -o exp2 -DEXPERIMENT_2

exp3: $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROGRAM).cpp -o exp3 -DEXPERIMENT_3

exp4: $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROGRAM).cpp -o exp4 -DEXPERIMENT_4

exp5: $(PROGRAM).cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(PROGRAM).cpp -o exp5 -DEXPERIMENT_5

run: $(PROGRAM)
	./$(PROGRAM)


clean:
	rm -f *.o $(PROGRAM) $(EXPERIMENTS)
