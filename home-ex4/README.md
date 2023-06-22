# Third Exercise in Operating System Course.

# === Description ===

This program emulates a memory allocation system run by an operating system.

# === files ===

The sim_mem.h defines the memory system class "sim_mem".

The sim_mem.cpp implements the sim_mem.h class's functions.

The main.cpp includes the sim_mem.h, and uses it to simulate store and load commands.
The one presented here is a simple example with multiple commands to check certain scenarios with this

# === sim_mem class ===

The sim_mem class represents the memory system.

To initialize the memory system you need to use the function:
sim_mem(<exe_file_name>, <swap_file_name>, <text_size>, <data_size>, <bss_size>, <heap_stack_size>, <page_size>)

<exe_file_name> - execution file name.
<swap_file_name> - swap file name.
<text_size> - text block size.
<data_size> - data block size.
<bss_size> - bss block size.
<heap_stack_size> - heap and stack block size.
<page_size> - the size of each page and frame.

store(<address>, <data>) - is used to store data in the memory block based on the address.

load(<address>) - is used to load data from a specific address.

<address> - represents a logical address in the system, has exactly 12 bits.

print_memory() - prints the current status of the main memory block.

void print_swap() - prints the current status of the swap file.

void print_page_table() - prints the current status of the page table.

# === How to compile ===

Simply run make in the directory, a makefile is provided.

# === How to run ===

Run the main executable file created by the makefile file.

# === Output ===

Depending on the main.cpp file used, should print out the last condition of the memory system.
