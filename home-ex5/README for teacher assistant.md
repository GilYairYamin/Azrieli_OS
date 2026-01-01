A simplified program simulating memory management of the unix operating system.
It recieves simple inputs and performs memory operations like creating files, writing, reading, and so on, based on how unix system do it today.
No directory management though, there is only one directory in this system.

To start the program simply compile the stub_code.cpp file and run the execution file resulted.

Basically there are 10 commands you can use:

0 - exit.\
1 - list all open file descriptors and print the content of the disk.\
2 - format the disk, will request the size of blocks in the formatted disk.\
3 - create a file, will request the name of the file, and will return its file descriptor.\
4 - open an existing file, will request the name of the file, and return its file descriptor.\
5 - close an open file, will request the file descriptor of the file.\
6 - write to an open file, will reuqest the file descriptor of the file and the string to write into it.\
7 - read from an open file, will request the file descriptor of the file and length of reading, will return the read content from the file.\
8 - delete a file which is not open, will request the name of the file.\
9 - copy a file, will request the name of the source file, and the name of the destination file.\
10 - change the name of a file, will request the current name, and the new name.

The prompts are very simplified, and if you accidentally put illegal input types (like a letter where the program expects a number), infinite loops and crashes may happen.
