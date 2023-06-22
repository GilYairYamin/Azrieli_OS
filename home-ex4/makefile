# Makefile
all: main.o sim_mem.o main clean

main: main.o sim_mem.o
	g++ -Wall -Wextra main.o sim_mem.o -o app

main.o: main.cpp sim_mem.h
	g++ -Wall -Wextra -c main.cpp

sim_mem.o: sim_mem.cpp sim_mem.h
	g++ -Wall -Wextra -c sim_mem.cpp

clean:
	rm -f *.o