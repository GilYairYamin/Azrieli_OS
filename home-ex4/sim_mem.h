
#ifndef SIM_MEM_H
#define SIM_MEM_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MEMORY_SIZE 200

extern char main_memory[MEMORY_SIZE];

class custom_queue {
private:
	int size;
	int values[MEMORY_SIZE];

public:
	int peek();
	void enqueue(int value);
	void remove(int value);
};

typedef struct page_descriptor {
	bool valid;
	bool dirty;
	int frame;
	int swap_index;
} page_descriptor;

class sim_mem {
	int swapfile_fd; //swap file fd
	int program_fd; //executable file fd

	int text_size;
	int data_size;
	int bss_size;
	int heap_stack_size;

	int page_size;
	int num_of_pages;

	int frame_mask;
	int inner_page_mask;
	int outer_page_mask;

	bool* used_frames_main_memory;
	bool* used_frames_swap;

	custom_queue* last_recently_used;
	page_descriptor** page_table; //pointer to page table

	int init_open_fds(char exe_file_name[], char swap_file_name[]);
	int init_alloc_memory();
	void init_masks();
	void destroy();

	bool is_valid_address(int outer, int inner, int offset);

	int setup_page(int outer, int inner, int op);

	int copy_page_from_exe(int outer, int inner);
	int bring_page_from_swap(int outer, int inner);
	int init_new_page(int outer, int inner);

	int find_page_using_frame(int frame, int& outer, int& inner);
	int swap_page_out(int frame);
	int find_empty_frame();
	int find_empty_swap_page();

	void update_page_table_added_to_memory(int outer, int inner, int frame);
	void update_page_table_swapped(int outer, int inner, int frame);
	void init_sizes_arr(int[]);

public:
	sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
		int data_size, int bss_size, int heap_stack_size,
		int page_size);

	char load(int address);
	void store(int address, char value);
	void print_memory();
	void print_swap();
	void print_page_table();

	~sim_mem();
};

#endif