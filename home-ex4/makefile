# Makefile
all: main.o sim_mem.o main clean

main: main.o sim_mem.o
	g++ -Wall -ggdb3 -Wextra main.o sim_mem.o -o main

main.o: main.cpp sim_mem.h
	g++ -Wall -ggdb3 -Wextra -c main.cpp

sim_mem.o: sim_mem.cpp sim_mem.h
	g++ -Wall -ggdb3 -Wextra -c sim_mem.cpp

clean:
	rm -f *.o