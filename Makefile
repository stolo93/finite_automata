CC = g++
CXXFLAGS = -Werror -Wextra -Wall
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
