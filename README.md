# Linux System Programming & OS Simulation Portfolio

[![Language](https://img.shields.io/badge/Language-C%20%7C%20C%2B%2B-blue.svg)](<https://en.wikipedia.org/wiki/C_(programming_language)>)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20POSIX-green.svg)](https://www.kernel.org/)
[![Build](https://img.shields.io/badge/Build-Make-orange.svg)](https://www.gnu.org/software/make/)

## 📖 Overview

This repository contains a comprehensive suite of low-level system components implemented from scratch. Together, these projects simulate the core subsystems of a modern Unix-like Operating System: **Process Management**, **Inter-Process Communication (IPC)**, **Memory Virtualization**, and **Persistent Storage**.

Each module demonstrates deep interaction with the Linux Kernel API, focusing on performance, synchronization, and resource safety.

## 🗂️ System Modules

### 1. Advanced Shell (Process Manager)

**Path:** `/ex2_shell`  
**Language:** C
A POSIX-compliant command-line interpreter that manages process lifecycles and job control.

- **Core Mechanics:** Uses `fork()`, `execvp()`, and `waitpid()` to execute commands.
- **Job Control:** Supports foreground/background execution (`&`), process pausing (`Ctrl+Z`), and resumption (`bg` command).
- **Pipeline:** Implements arbitrary command chaining (`ls | grep | wc`) using `pipe()` and `dup2()` for I/O redirection.
- **Signal Handling:** Custom handlers for `SIGTSTP` and `SIGINT` to manage process groups without killing the shell.

### 2. High-Performance IPC System

**Path:** `/ex3_concurrency`  
**Language:** C
A parallel processing engine utilizing the Producer-Consumer pattern to offload complex polynomial calculations.

- **Shared Memory:** Implements zero-copy data transfer between processes using System V Shared Memory.
- **Synchronization:** Uses **POSIX Semaphores** for process-level locking and **Pthread Mutexes** for thread-level safety.
- **Multithreading:** The consumer process spawns worker threads to calculate polynomial coefficients in parallel, maximizing CPU throughput.

### 3. Virtual Memory Management Unit (MMU)

**Path:** `/ex4_virtual_memory`  
**Language:** C++
A simulation of a hardware MMU handling logical-to-physical address translation.

- **Paging:** Implements a two-level hierarchical Page Table structure.
- **Demand Paging:** Loads pages from the executable only when accessed (lazy loading).
- **Swap Management:** Handles page faults by evicting pages to a simulated disk (swap file) based on a **Least Recently Used (LRU)** algorithm.
- **Write-Back Policy:** Optimizes I/O by only writing "dirty" pages back to the swap file.

### 4. Inode-based File System

**Path:** `/ex5_filesystem`  
**Language:** C++
A user-space file system implementing the Unix Inode architecture for persistent storage.

- **Inode Structure:** Supports Direct, Single Indirect, and Double Indirect block pointers to handle varying file sizes.
- **Disk Management:** Uses a **BitVector** for efficient O(1) free-block lookup and allocation.
- **Persistence:** Simulates a physical disk drive using a binary file, ensuring data persists between sessions.
- **Operations:** Full support for `Create`, `Read`, `Write`, `Delete` (with deep block reclamation), `Rename`, and `Copy`.

---

## 🛠️ Technical Competencies

This portfolio demonstrates proficiency in the following System Engineering concepts:

| Category         | Concepts Applied                                                       |
| :--------------- | :--------------------------------------------------------------------- |
| **Concurrency**  | Race Conditions, Deadlocks, Mutexes, Semaphores, Atomic Operations     |
| **OS Internals** | System Calls, Context Switching, Signals, File Descriptors, Inodes     |
| **Memory**       | Pointers, Malloc/Free, Buffer Overflows, Virtual Addressing, Paging    |
| **Algorithms**   | LRU Caching, BitVector Allocation, Tree Traversal (Indirect Blocks)    |
| **Tools**        | GDB (Debugging), Valgrind (Memory Leaks), Makefiles (Build Automation) |

## 🚀 Build & Run

Each module is self-contained. To build a specific module, navigate to its directory and run `make`.

**Example (Building the File System):**

```bash
cd ex5_filesystem
make all
./fs_sim
```
