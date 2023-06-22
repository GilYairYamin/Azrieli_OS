
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
	int swapfile_fd;		//swap file fd
	int program_fd;			//executable file fd

	int text_size;			// size of text portion
	int data_size;			// size of data portion
	int bss_size;			// size of vss portion
	int heap_stack_size;	// size of heap and stack portion

	int page_size;			// size of a single page in the system.
	int num_of_pages;		// the number of pages in the system.

	int outer_page_mask;	// outer page mask
	int inner_page_mask;	// inner page mask
	int frame_offset_mask;	// frame offset mask

	bool* used_frames_main_memory;	// boolean array representing currently being used portions of memory.
	bool* used_frames_swap;			// boolean array representing currently being used portions of swap file.

	custom_queue* last_recently_used;	// queue of last recently page currently in memory
	page_descriptor** page_table;		// pointer to page table

	/**
	 * @brief open files and initialize file descriptors for exec file and swap file.
	 *
	 * @param exe_file_name
	 * @param swap_file_name
	 * @return int
	 */
	int init_open_fds(char exe_file_name[], char swap_file_name[]);

	/**
	 * @brief allocate all objects needed in memory including arrays and page table.
	 *
	 * @return int
	 */
	int init_alloc_memory();

	/**
	 * @brief initilize outer_page_mask, inner_page_mask, frame_offset_mask.
	 *
	 */
	void init_masks();

	/**
	 * @brief deallocate all memory, close all file descriptors.
	 *
	 */
	void destroy();

	/**
	 * @brief check if an outer, inner, offset of an address is valid.
	 *
	 */
	bool is_valid_address(int outer, int inner, int offset);

	/**
	 * @brief Make sure a page is set up correctly in the main memory.
	 *
	 */
	int setup_page(int outer, int inner, int op);

	/**
	 * @brief copy a page from the execution file.
	 *
	 */
	int copy_page_from_exe(int outer, int inner);
	/**
	 * @brief bring a page from the swap file.
	 *
	 */
	int bring_page_from_swap(int outer, int inner);
	/**
	 * @brief initialize a new page.
	 *
	 */
	int init_new_page(int outer, int inner);


	/**
	 * @brief find a page's outer index and inner index by the frame.
	 *
	 */
	int find_page_using_frame(int frame, int& outer, int& inner);
	/**
	 * @brief swap a page, currently occuping a specific frame
	 *
	 */
	int swap_page_out(int frame);
	/**
	 * @brief return an empty frame in the main memory block.
	 *
	 */
	int find_empty_frame();
	/**
	 * @brief return an empty frame in the swap file.
	 *
	 */
	int find_empty_swap_frame();

	/**
	 * @brief update page_table with a new page added to the main memory
	 *
	 */
	void update_page_table_added_to_memory(int outer, int inner, int frame);
	/**
	 * @brief update page_table with a new page added to the swap file
	 *
	 */
	void update_page_table_swapped(int outer, int inner, int frame);
	/**
	 * @brief initialize an array with the sizes of each inner page table.
	 *
	 */
	void init_sizes_arr(int arr[]);

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