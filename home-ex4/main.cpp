#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

int main() {
	char exec_file[100] = "exec_file";
	char swap_file[100] = "swap_file";

	sim_mem mem_sm(exec_file, swap_file, 128, 128, 128, 128, 64);

	// for (int i = 0; i < 128; i += 64) {
	// 	mem_sm.store(2048 + i, 'X');
	// }
	// for (int i = 0; i < 128; i += 64) {
	// 	mem_sm.store(3073 + i, 'X');
	// }

	mem_sm.store(0, 'X');
	char v = mem_sm.load(3072);
	printf("value: %c\n", v);

	mem_sm.load(0);
	mem_sm.load(64);
	mem_sm.load(1024);
	mem_sm.load(1088);
	mem_sm.load(2048);
	mem_sm.load(2112);

	mem_sm.store(3072, 'A');
	mem_sm.store(3136, 'A');
	mem_sm.store(1024, 'B');
	mem_sm.store(1088, 'B');
	mem_sm.store(2048, 'C');
	mem_sm.store(2112, 'C');

	mem_sm.load(0);
	mem_sm.load(64);
	mem_sm.load(1024);
	mem_sm.load(1088);
	mem_sm.load(2048);
	mem_sm.load(2112);

	// mem_sm.store(3072, 'B');
	// mem_sm.store(3136, 'B');

	mem_sm.print_memory();
	mem_sm.print_swap();
	mem_sm.print_page_table();
}