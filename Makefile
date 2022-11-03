CC = g++
CXXFLAGS = -Werror -Wextra -Wall -Wcatch-value=0 #Had to add this because of warning from args.h
PROG_NAME = automata
OBJS = main.o FiniteAutomata.o
SOURCES = src/*.cpp
VPATH = src:

all: dependencies main

main: $(OBJS)
	$(CC) -o $(PROG_NAME) $^

dependencies:
	$(CC) -MM $(SOURCES) > $@

-include dependencies

clean:
	rm *.o automata dependencies

.PHONY: all main dependencies clean
