# Custom Linux Shell Implementation

A lightweight, custom-built command-line interface (CLI) shell for Linux, implemented in C. This project demonstrates core Operating System concepts including process creation, memory management, and string parsing.

## 🚀 Features

* **Command Execution:** Executes standard Linux commands (e.g., `ls`, `grep`, `sleep`) using system calls.
* **Sequential Execution:** Supports chaining multiple commands in a single line using the `;` delimiter.
* **Variable Management:**
    * Dynamic creation and storage of session variables (e.g., `x=10`).
    * Variable expansion in commands (e.g., `echo $x`).
    * Memory automatically expands to accommodate new variables using `realloc`.
* **Argument Parsing:** Handles complex argument splitting, including preserving whitespace within double quotes (`" "`).
* **Informative Prompt:** Displays real-time session statistics:
    * Count of successful commands executed.
    * Total number of arguments processed.
    * Current Working Directory.
* **Graceful Exit:** Detects 3 consecutive empty inputs (Enter key) to terminate the session.

## 🛠 Technical Implementation

### System Calls & Process Management
The shell utilizes the `fork()` and `execvp()` system calls to create child processes for command execution. It implements a `wait()` strategy to ensure the parent process (the shell) pauses until the command execution is complete.

### Dynamic Memory Management
To handle an arbitrary number of variables without memory waste, the project implements a dynamic array structure.
* **Structs:** Uses a `GlobalVariables` struct to track state.
* **Resizing:** When the variable limit is reached, the memory block is resized using `realloc` (doubling capacity) to ensure O(1) amortized insertion time.
* **Cleanup:** Includes a rigorous `freeGlobalVars` routine to ensure no memory leaks occur upon shell exit or fatal errors.

### Parsing Logic
A custom parser (`splitStrIgnoreQuotes`) was implemented to tokenize input strings. It manually iterates through pointers to handle edge cases like:
* Distinguishing between command separators (`;`) and argument spaces.
* Ignoring delimiters strictly when they appear inside quote blocks.

## 💻 Building and Running

### Prerequisites
* GCC Compiler
* Linux Environment (or WSL)

### Compilation
If the source is split into modules:
```bash
gcc -o myshell main.c commands.c global_variables.c