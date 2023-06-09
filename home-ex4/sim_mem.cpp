#include "sim_mem.h"

#define SUCCESS 0
#define ERROR 1

#define DEFAULT_SWAP_INDEX -1

#define TEXT_INDEX 0
#define DATA_INDEX 1
#define BSS_INDEX 2
#define STACK_HEAP_INDEX 3

#define OUTER_PAGE_AMOUNT 4

#define INNER_PAGE_MASK_CALC 1023
#define OUTER_PAGE_MASK 3072

#define LOAD_OP 0
#define STORE_OP 1

void custom_queue::enqueue(int value) {
	this->remove(value);
	if (this->size >= MEMORY_SIZE)
		return;

	this->values[this->size++] = value;
}

void custom_queue::remove(int value) {
	int index = -1;
	for (int i = 0; i < this->size; i++) {
		if (this->values[i] == value) {
			index = i;
			break;
		}
	}

	if (index < 0)
		return;

	this->size--;
	for (int i = index; i < this->size; i++) {
		this->values[i] = this->values[i + 1];
	}
}

int custom_queue::peek() {
	if (this->size <= 0)
		return -1;

	return this->values[0];
}

int get_value_from_mask(int address, int mask) {
	address = address & mask;
	while (mask > 0 && (mask & 1) == 0) {
		mask = mask >> 1;
		address = address >> 1;
	}
	return address;
}

void sim_mem::init_sizes_arr(int arr[]) {
	arr[TEXT_INDEX] = this->text_size / this->page_size;
	arr[DATA_INDEX] = this->data_size / this->page_size;
	arr[BSS_INDEX] = this->bss_size / this->page_size;
	arr[STACK_HEAP_INDEX] = this->heap_stack_size / this->page_size;
}

int sim_mem::init_alloc_memory() {
	int nums[OUTER_PAGE_AMOUNT] = { 0 };
	this->init_sizes_arr(nums);

	this->page_table = (page_descriptor**)calloc(sizeof(page_descriptor*), OUTER_PAGE_AMOUNT);
	if (this->page_table == NULL) {
		perror("memory allocation error - outer page table");
		return ERROR;
	}

	for (int outer = 0; outer < OUTER_PAGE_AMOUNT; outer++) {
		this->page_table[outer] = (page_descriptor*)calloc(sizeof(page_descriptor), nums[outer]);
		if (this->page_table[outer] == NULL) {
			perror("memory allocation error - inner page table\n");
			return ERROR;
		}
	}

	this->used_frames_main_memory = (bool*)calloc(sizeof(bool), MEMORY_SIZE / this->page_size);
	if (this->used_frames_main_memory == NULL) {
		perror("memory allocation error - available frames\n");
		return ERROR;
	}

	int num_of_max_pages_in_swap = (this->data_size + this->bss_size + this->heap_stack_size) / this->page_size;
	this->used_frames_swap = (bool*)calloc(sizeof(bool), num_of_max_pages_in_swap);
	if (this->used_frames_main_memory == NULL) {
		perror("memory allocation error - available frames\n");
		return ERROR;
	}

	this->last_recently_used = (custom_queue*)calloc(sizeof(custom_queue), 1);

	if (this->last_recently_used == NULL) {
		perror("memory allocation error - least recently used\n");
		return ERROR;
	}
	return SUCCESS;
}

int sim_mem::init_open_fds(char exe_file_name[], char swap_file_name[]) {
	this->program_fd = open(exe_file_name, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
	if (this->program_fd < 0) {
		perror("couldn't open executable file\n");
		return ERROR;
	}

	this->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, 0666);
	if (this->swapfile_fd < 0) {
		perror("couldn't open program file\n");
		return ERROR;
	}

	return SUCCESS;
}

void sim_mem::init_masks() {
	this->outer_page_mask = OUTER_PAGE_MASK;
	this->inner_page_mask = 0;
	this->frame_offset_mask = 0;

	int page_size = this->page_size - 1;
	int amount = 0;
	while (page_size > 0) {
		amount++;
		page_size = page_size >> 1;
	}

	amount = (amount < 1) ? 1 : amount;
	while (amount-- > 0) {
		this->frame_offset_mask = (this->frame_offset_mask << 1) + 1;
	}

	this->inner_page_mask = (this->frame_offset_mask ^ INNER_PAGE_MASK_CALC) & INNER_PAGE_MASK_CALC;
}

void sim_mem::destroy() {
	if (this->program_fd > -1) {
		close(this->program_fd);
	}
	if (this->swapfile_fd > -1) {
		close(this->swapfile_fd);
	}

	if (this->page_table != NULL) {
		for (int outer = 0; outer < OUTER_PAGE_AMOUNT; outer++) {
			free(this->page_table[outer]);
		}
	}

	free(this->page_table);
	free(this->used_frames_main_memory);
	free(this->used_frames_swap);
	free(this->last_recently_used);
}

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size,
	int data_size, int bss_size, int heap_stack_size,
	int page_size) {

	this->text_size = text_size;
	this->data_size = data_size;
	this->bss_size = bss_size;
	this->heap_stack_size = heap_stack_size;
	this->page_size = page_size;

	this->program_fd = -1;
	this->swapfile_fd = -1;

	this->page_table = NULL;
	this->used_frames_main_memory = NULL;
	this->used_frames_swap = NULL;
	this->last_recently_used = NULL;

	this->num_of_pages = 0;

	this->init_masks();

	if (this->init_open_fds(exe_file_name, swap_file_name) != 0) {
		this->destroy();
		exit(1);
	}

	if (this->init_alloc_memory() != 0) {
		this->destroy();
		exit(1);
	}

	memset(main_memory, 0, MEMORY_SIZE);

	int num_of_max_pages_in_swap = (this->data_size + this->bss_size + this->heap_stack_size) / this->page_size;

	lseek(this->swapfile_fd, 0, SEEK_SET);
	for (int i = 0; i < num_of_max_pages_in_swap; i++) {
		if (write(this->swapfile_fd, main_memory, this->page_size) < 0) {
			perror("writing error to swap file\n");
		}
	}
}

sim_mem::~sim_mem() {
	this->destroy();
}

int sim_mem::find_page_using_frame(int frame, int& outer, int& inner) {
	int nums[OUTER_PAGE_AMOUNT] = { 0 };
	this->init_sizes_arr(nums);

	page_descriptor* p = NULL;
	for (int o = 0; o < OUTER_PAGE_AMOUNT; o++) {
		for (int i = 0; i < nums[o]; i++) {
			p = &this->page_table[o][i];
			if (p->valid == true && p->frame == frame) {
				outer = o;
				inner = i;
				return SUCCESS;
			}
		}
	}
	return ERROR;
}

int sim_mem::swap_page_out(int frame) {
	int outer = 0;
	int inner = 0;
	if (this->find_page_using_frame(frame, outer, inner)) {
		fprintf(stderr, "couldn't find page using frame\n");
		return ERROR;
	}

	if (outer == TEXT_INDEX) {
		this->update_page_table_swapped(outer, inner, -1);
		return SUCCESS;
	}

	int swap_frame = this->find_empty_swap_frame();
	if (swap_frame < 0) {
		fprintf(stderr, "couldn't find empty swap page\n");
		return ERROR;
	}

	int swap_offset = swap_frame * this->page_size;
	int main_offset = this->page_table[outer][inner].frame * this->page_size;

	lseek(this->swapfile_fd, swap_offset, SEEK_SET);
	if (write(this->swapfile_fd, &main_memory[main_offset], this->page_size) < 0) {
		perror("writing error to swap file\n");
		return ERROR;
	}

	this->update_page_table_swapped(outer, inner, swap_frame);
	return SUCCESS;
}

int sim_mem::find_empty_swap_frame() {
	int num_of_max_pages_in_swap = (this->data_size + this->bss_size + this->heap_stack_size) / this->page_size;

	for (int i = 0; i < num_of_max_pages_in_swap; i++) {
		if (this->used_frames_swap[i] == false)
			return i;
	}

	fprintf(stderr, "couldn't find swap place? how?\n");
	return -1;
}

int sim_mem::find_empty_frame() {
	int frame_amount = MEMORY_SIZE / this->page_size;

	for (int i = 0; i < frame_amount; i++) {
		if (this->used_frames_main_memory[i] == false) {
			return i;
		}
	}

	int frame = this->last_recently_used->peek();
	if (frame < 0) {
		fprintf(stderr, "there is no free frame in memory AND the queue of used frames is empty, should NEVER get to this error.\n");
		return -1;
	}

	if (this->swap_page_out(frame)) {
		return -1;
	}

	return frame;
}

void sim_mem::update_page_table_swapped(int outer, int inner, int swap_frame) {
	int frame = this->page_table[outer][inner].frame;

	this->page_table[outer][inner].valid = false;
	this->page_table[outer][inner].dirty = true;
	this->page_table[outer][inner].frame = 0;
	this->page_table[outer][inner].swap_index = swap_frame;

	if (swap_frame < 0)
		this->page_table[outer][inner].dirty = false;
	else
		this->used_frames_swap[swap_frame] = true;

	this->used_frames_main_memory[frame] = false;
	this->last_recently_used->remove(frame);
}

void sim_mem::update_page_table_added_to_memory(int outer, int inner, int frame) {
	int swap_frame = this->page_table[outer][inner].swap_index;

	this->page_table[outer][inner].valid = true;
	this->page_table[outer][inner].dirty = false;
	this->page_table[outer][inner].frame = frame;
	this->page_table[outer][inner].swap_index = DEFAULT_SWAP_INDEX;

	this->used_frames_main_memory[frame] = true;
	if (swap_frame >= 0)
		this->used_frames_swap[swap_frame] = false;

	this->last_recently_used->enqueue(frame);
}

int sim_mem::copy_page_from_exe(int outer, int inner) {
	int empty_frame = this->find_empty_frame();
	if (empty_frame < 0) {
		fprintf(stderr, "couldn't find an empty frame, should NEVER get to this error.\n");
		return ERROR;
	}

	int swap_offset = (outer == TEXT_INDEX) ? 0 : this->text_size;
	swap_offset += inner * this->page_size;
	int main_offset = empty_frame * this->page_size;

	lseek(this->program_fd, swap_offset, SEEK_SET);
	if (read(this->program_fd, &main_memory[main_offset], this->page_size) < 0) {
		perror("reading error from execution file\n");
		return ERROR;
	}

	this->page_table[outer][inner].swap_index = -1;
	this->update_page_table_added_to_memory(outer, inner, empty_frame);
	return SUCCESS;
}

int sim_mem::bring_page_from_swap(int outer, int inner) {
	int empty_frame = this->find_empty_frame();
	if (empty_frame < 0) {
		fprintf(stderr, "what the heck?!\n");
		return ERROR;
	}

	int swap_offset = page_table[outer][inner].swap_index * this->page_size;
	int main_offset = empty_frame * this->page_size;

	lseek(this->swapfile_fd, swap_offset, SEEK_SET);
	if (read(this->swapfile_fd, &main_memory[main_offset], this->page_size) < 0) {
		perror("reading error from swap file\n");
		return ERROR;
	}
	this->update_page_table_added_to_memory(outer, inner, empty_frame);

	char zeroByte[MEMORY_SIZE] = { 0 };
	lseek(this->swapfile_fd, swap_offset, SEEK_SET);
	if (write(this->swapfile_fd, zeroByte, this->page_size) < 0) {
		perror("writing error to swap file\n");
	}
	return SUCCESS;
}

int sim_mem::init_new_page(int outer, int inner) {
	int empty_frame = this->find_empty_frame();
	if (empty_frame < 0) {
		fprintf(stderr, "what the heck?!\n");
		return ERROR;
	}

	int main_offset = empty_frame * this->page_size;
	memset(&main_memory[main_offset], 0, this->page_size);

	this->page_table[outer][inner].swap_index = DEFAULT_SWAP_INDEX;
	this->update_page_table_added_to_memory(outer, inner, empty_frame);

	return SUCCESS;
}

int sim_mem::setup_page(int outer, int inner, int op) {
	if (page_table[outer][inner].valid) {
		this->last_recently_used->enqueue(page_table[outer][inner].frame);
		return SUCCESS;
	}

	if (outer == TEXT_INDEX) {
		if (op == STORE_OP) {
			fprintf(stderr, "attempt to write to exec file\n");
			return ERROR;
		}
		return this->copy_page_from_exe(outer, inner);
	}

	if (page_table[outer][inner].dirty) {
		return this->bring_page_from_swap(outer, inner);
	}

	if (outer == DATA_INDEX) {
		return this->copy_page_from_exe(outer, inner);
	}

	if (outer == BSS_INDEX) {
		return this->init_new_page(outer, inner);
	}

	if (outer == STACK_HEAP_INDEX) {
		if (op == LOAD_OP) {
			fprintf(stderr, "attempt to read from uninitialized memory\n");
			return ERROR;
		}
		return this->init_new_page(outer, inner);
	}

	fprintf(stderr, "unknown error, what?!\n");
	return ERROR;
}

bool sim_mem::is_valid_address(int outer, int inner, int offset) {
	if (outer < 0 || outer >= OUTER_PAGE_AMOUNT) {
		fprintf(stderr, "illegal outer address.\n");
		return false;
	}

	int nums[OUTER_PAGE_AMOUNT] = { 0 };
	this->init_sizes_arr(nums);

	if (inner < 0 || inner >= nums[outer]) {
		fprintf(stderr, "illegal inner address.\n");
		return false;
	}

	if (offset < 0 || offset >= this->page_size) {
		fprintf(stderr, "illegal offset address.\n");
		return false;
	}

	return true;
}

char sim_mem::load(int address) {
	int outer = get_value_from_mask(address, this->outer_page_mask);
	int inner = get_value_from_mask(address, this->inner_page_mask);
	int frame_offset = get_value_from_mask(address, this->frame_offset_mask);

	if (!this->is_valid_address(outer, inner, frame_offset)) {
		return '\0';
	}

	if (this->setup_page(outer, inner, LOAD_OP)) {
		return '\0';
	}

	int frame_start = this->page_table[outer][inner].frame * this->page_size;
	return main_memory[frame_start + frame_offset];
}

void sim_mem::store(int address, char value) {
	int outer = get_value_from_mask(address, this->outer_page_mask);
	int inner = get_value_from_mask(address, this->inner_page_mask);
	int frame_offset = get_value_from_mask(address, this->frame_offset_mask);

	if (!this->is_valid_address(outer, inner, frame_offset)) {
		return;
	}

	if (this->setup_page(outer, inner, STORE_OP)) {
		return;
	}

	int frame_start = this->page_table[outer][inner].frame * this->page_size;
	main_memory[frame_start + frame_offset] = value;
}

/**************************************************************************************/
void sim_mem::print_memory() {
	int i;
	printf("\n Physical memory\n");
	for (i = 0; i < MEMORY_SIZE; i += this->page_size) {
		for (int j = 0; j < MEMORY_SIZE && j < this->page_size; j++) {
			printf("[%c]\t", main_memory[i + j]);
		}
		printf("\n");
	}
}

/************************************************************************************/
void sim_mem::print_swap() {
	char* str = (char*)malloc(this->page_size * sizeof(char));
	int i;
	printf("\n Swap memory\n");
	lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file

	int x = read(swapfile_fd, str, this->page_size);
	while (x == this->page_size) {
		for (i = 0; i < page_size; i++) {
			printf("[%c]\t", str[i]);
		}
		printf("\n");
		x = read(swapfile_fd, str, this->page_size);
	}

	if (x < 0) {
		perror("attempt to read from swap file failed");
	}
}

/***************************************************************************************/
void sim_mem::print_page_table() {
	int i;
	int num_of_txt_pages = text_size / page_size;
	int num_of_data_pages = data_size / page_size;
	int num_of_bss_pages = bss_size / page_size;
	int num_of_stack_heap_pages = heap_stack_size / page_size;

	printf("Valid\t Dirty\t Frame\t Swap index\n");
	for (i = 0; i < num_of_txt_pages; i++) {
		printf("[%d]\t[%d]\t[%d]\t[%d]\n",
			page_table[0][i].valid,
			page_table[0][i].dirty,
			page_table[0][i].frame,
			page_table[0][i].swap_index);

	}

	printf("Valid\t Dirty\t Frame\t Swap index\n");
	for (i = 0; i < num_of_data_pages; i++) {
		printf("[%d]\t[%d]\t[%d]\t[%d]\n",
			page_table[1][i].valid,
			page_table[1][i].dirty,
			page_table[1][i].frame,
			page_table[1][i].swap_index);

	}

	printf("Valid\t Dirty\t Frame\t Swap index\n");
	for (i = 0; i < num_of_bss_pages; i++) {
		printf("[%d]\t[%d]\t[%d]\t[%d]\n",
			page_table[2][i].valid,
			page_table[2][i].dirty,
			page_table[2][i].frame,
			page_table[2][i].swap_index);

	}

	printf("Valid\t Dirty\t Frame\t Swap index\n");
	for (i = 0; i < num_of_stack_heap_pages; i++) {
		printf("[%d]\t[%d]\t[%d]\t[%d]\n",
			page_table[3][i].valid,
			page_table[3][i].dirty,
			page_table[3][i].frame,
			page_table[3][i].swap_index);
	}
}