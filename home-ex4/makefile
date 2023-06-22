# Makefile

CC = g++
CFLAGS = -Wall -Wextra

# List your source files here
SRCS = main.cpp sim_mem.cpp

# Derive the object file names from the source file names
OBJS = $(SRCS:.cpp=.o)

# Specify the name of the final executable
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Rule to compile each source file into an object file
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)