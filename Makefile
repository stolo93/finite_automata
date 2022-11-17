CC = g++
CXXFLAGS = -Wextra -Wall -Wcatch-value=0 #Had to add this because of warning from args.h
PROG_NAME = automata
OBJS = main.o FiniteAutomata.o simlib.o
SOURCES = src/*.cpp
VPATH = src:

all: dependencies main

main: $(OBJS)
	$(CC) -o $(PROG_NAME) $^

simlib.o:
	g++ -c -Wall -Wfloat-equal -fms-extensions -fdiagnostics-show-option -std=c++14 -Wctor-dtor-privacy -Weffc++ -fPIC -fno-strict-aliasing src/3rd_party/explicit_lts_sim.cc -o $@

dependencies:
	$(CC) -MM $(SOURCES) > $@

-include dependencies

clean:
	rm *.o automata dependencies

.PHONY: all main dependencies clean
