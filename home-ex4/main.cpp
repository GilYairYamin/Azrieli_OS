#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

int main() {
	char val;
	char exec_file[100] = "exec_file";
	char swap_file[100] = "swap_file";

	sim_mem mem_sm(exec_file, swap_file, 128, 128, 128, 128, 32);

	for (int i = 0; i < 128; i += 32) {
		mem_sm.store(2048 + i, 'X');
	}
	for (int i = 0; i < 128; i += 32) {
		mem_sm.store(3073 + i, 'X');
	}
	// mem_sm.store(2080, 'X');
	// mem_sm.store(2081, 'X');
	// mem_sm.store(2082, 'X');
	// mem_sm.store(2083, 'X');
	// mem_sm.store(2084, 'X');
	// mem_sm.store(2085, 'X');
	// mem_sm.store(2086, 'X');
	// mem_sm.store(2087, 'X');
	// mem_sm.store(2088, 'X');
	// mem_sm.store(2089, 'X');
	// mem_sm.store(2090, 'X');
	// mem_sm.store(2091, 'X');
	// mem_sm.store(2092, 'X');
	// mem_sm.store(2112, 'X');
	// mem_sm.store(2128, 'X');
	// mem_sm.store(2144, 'X');
	// mem_sm.store(2160, 'X');
	// mem_sm.store(2161, 'X');
	// mem_sm.store(2161, 'X');
	mem_sm.store(2048, 'B');
	// mem_sm.store(2066, 'X');
	val = mem_sm.load(0);
	val = mem_sm.load(1);
	// val = mem_sm.load(2);
	// val = mem_sm.load(16);
	val = mem_sm.load(32);
	// val = mem_sm.load(48);
	val = mem_sm.load(64);
	mem_sm.store(2048 + 32, 'B');
	// val = mem_sm.load(80);
	// val = mem_sm.load(96);
	// val = mem_sm.load(112);

	printf("%c\n", val);
	mem_sm.print_memory();
	mem_sm.print_swap();
	mem_sm.print_page_table();
}