# Virtual Memory Management Simulator (MMU)

A low-level C++ simulation of a Memory Management Unit (MMU), implementing Demand Paging, Two-Level Page Tables, and Swap mechanisms. This system intercepts logical memory accesses and translates them to physical addresses, handling page faults and disk I/O transparently.

## 🚀 Key Features

- **Address Translation:** Implements bitwise logic to decompose logical addresses into **Outer Page**, **Inner Page**, and **Offset** components using dynamic bitmasks.
- **Demand Paging:** Loads pages lazily from the executable file (text/data) or swap file (stack/heap) only when accessed.
- **Page Replacement Policy:** Implements a custom **Least Recently Used (LRU)** queue to manage frame eviction when physical memory is full.
- **Memory Protection:** Enforces Read-Only permissions for the Text segment and detects illegal access attempts.
- **Swap Management:** Handles "dirty" page eviction (write-back policy), persisting modified data to a swap file using POSIX file descriptors.

## 🛠 Technical Architecture

### The Page Table

The system uses a hierarchical (two-level) page table structure to map logical addresses to physical frames.

- **Valid Bit:** Indicates if the page is currently in RAM.
- **Dirty Bit:** Tracks if the page has been modified (requires write-back to swap).
- **Frame Mapping:** Stores the physical frame index.

### The Workflow

1.  **Intercept:** `load(addr)` or `store(addr, val)` is called.
2.  **Translate:** The address is masked to find the specific Page Table Entry (PTE).
3.  **Check:**
    - _Hit:_ If `valid=1`, access physical RAM immediately.
    - _Miss (Page Fault):_ If `valid=0`, trigger the Page Fault Handler.
4.  **Handle Fault:**
    - Find a free frame (or evict one using **LRU**).
    - If evicting a "dirty" page, write it to the `swap_file`.
    - Read the requested page from `exec_file` or `swap_file`.
    - Update the Page Table and retry the access.

## 💻 Usage

### Compilation

```bash
make all
```
