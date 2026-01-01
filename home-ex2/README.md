# Advanced Linux Shell Implementation (C)

A fully functional, POSIX-compliant shell that handles complex process management, inter-process communication (IPC), and job control. This project demonstrates deep understanding of the Linux kernel interface, file descriptors, and signal handling.

## 🚀 Key Features

* **Piping & Redirection:** Supports arbitrary command chaining using pipes (`|`) and file output redirection (`>`).
* **Job Control:**
    * Run commands in background (`&`).
    * Pause foreground processes (Ctrl+Z / `SIGTSTP`).
    * Resume paused processes in the background (`bg`).
* **Signal Handling:** Custom handlers for process state changes, ensuring zombie processes are reaped correctly.
* **Environment Management:** Supports setting/getting local session variables (`x=value`, `$x`).

## 🛠 Technical Architecture

### Inter-Process Communication (IPC)
The shell orchestrates data flow between processes using kernel pipes.
* **Implementation:** Uses `pipe()` to create channels and `dup2()` to manipulate process file descriptors (replacing `STDIN`/`STDOUT` dynamically).
* **Synchronization:** Coordinates parent/child execution flows to prevent race conditions during read/write operations.

### Process Groups & Job Control
Unlike basic shells, this implementation manages **Process Group IDs (PGIDs)**.
* **Foreground/Background:** Uses `setpgid()` to isolate background jobs, preventing them from receiving terminal signals intended for the foreground process.
* **Signal Forwarding:** Captures `SIGTSTP` to pause groups of processes and manages a stack of paused jobs for resumption.

### Memory & Resource Safety
* **No Memory Leaks:** Implements a strict singleton pattern for global state `GlobalVariables` with full tear-down logic.
* **Descriptor Management:** Includes a `safelyCloseFileDscriptor` wrapper to ensure file descriptors are never leaked, preventing resource exhaustion.

## 💻 Usage

### Compilation
```bash
gcc -o myshell ex2.c