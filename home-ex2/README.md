# Second Exercise in Operating System Course.

# === Description ===

This program is a simplified shell made for the linux operating system as an exercise in Operating System concepts.

Using low level c programming to implement:
Multiple processes running parrallel to each other.
Processes interacting with each other using pipes and signals.
And processes interacting with the files of the system.

The program supports:
Most of linux's built in commands such as echo, ls, wc, and more.
Multiple concurrent commands, the operator ';' marks the end of a command.
Running multiple commands in the background, marked with '&' at the end of a command instead of ';'.
Saving global variables using '<name>=<value>' syntax.
Using a global variable with '$<name>' syntax.
Saving to files using the '>' operator.
Piping multiple commands using the '|' operator.
Pausing a command using Control+Z key combination.
Resuming the operation of a command previously paused with the command bg.

# === Program files ===

ex2.c - the main file with all the code in it.

# === How to compile ===

All the code is saved in the file ex2.c, therefore to make it use any compiler you want and simply compile that file into an executable for linux.
Note: will not work on windows.

example:
gcc ex2.c -o ex2.bin

# === Input ===

Most of the standard terminal commands in linux, such as 'echo', 'ls', 'wc', 'man' and so on.
<name>=<value> strings for defining global variables.

Specifically doesn't support the commands 'cd' and 'fg'.

# === Output ===

The path of the current folder, amount of commands and arguments successfully executed and the standard output of each command used.
