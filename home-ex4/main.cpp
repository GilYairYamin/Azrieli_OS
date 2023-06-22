#include "sim_mem.h"

char main_memory[MEMORY_SIZE];

#define EXEC_FILE_NAME "exec_file"
#define SWAP_FILE_NAME "swap_file"

#define TEXT_INDEX 0
#define DATA_INDEX 1
#define BSS_INDEX 2
#define STACK_HEAP_INDEX 3

#define TEXT_SIZE 128
#define DATA_SIZE 128
#define BSS_SIZE 128
#define STACK_HEAP_SIZE 128

#define PAGE_SIZE 8

#define TEXT_MASK 0
#define DATA_MASK 1024
#define BSS_MASK 2048
#define STACK_HEAP_MASK 3072

int outer_sizes[4] = { TEXT_SIZE , DATA_SIZE , BSS_SIZE , STACK_HEAP_SIZE };
int outer_masks[4] = { TEXT_MASK , DATA_MASK , BSS_MASK , STACK_HEAP_MASK };

int create_address(int outer, int inner, int offset) {
	if (outer < 0 || outer > 3)
		return -1;

	int inner_amount = outer_sizes[outer] / PAGE_SIZE;
	int res = outer_masks[outer];
	res += (inner % inner_amount) * PAGE_SIZE;
	res += offset % PAGE_SIZE;
	return res;
}

int main() {
	char exec_file[200] = EXEC_FILE_NAME;
	char swap_file[200] = SWAP_FILE_NAME;

	sim_mem mem_sm(exec_file, swap_file, TEXT_SIZE, DATA_SIZE, BSS_SIZE, STACK_HEAP_SIZE, PAGE_SIZE);

	mem_sm.store(0, 'X');
	mem_sm.load(3072);

	mem_sm.store(create_address(3, 0, 1), 'X');

	mem_sm.load(create_address(0, 0, 0));
	mem_sm.load(create_address(0, 1, 0));
	mem_sm.load(create_address(0, 2, 0));
	mem_sm.load(create_address(0, 3, 0));
	mem_sm.load(create_address(1, 0, 0));
	mem_sm.load(create_address(1, 1, 0));
	mem_sm.load(create_address(1, 2, 0));
	mem_sm.load(create_address(1, 3, 0));
	mem_sm.load(create_address(2, 0, 0));
	mem_sm.load(create_address(2, 1, 0));
	mem_sm.load(create_address(2, 2, 0));
	mem_sm.load(create_address(2, 3, 0));

	char v = mem_sm.load(create_address(3, 0, 1));
	printf("%c\n", v);

	mem_sm.print_memory();
	mem_sm.print_swap();
	mem_sm.print_page_table();
}