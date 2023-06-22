# Third Exercise in Operating System Course.

# === Description ===

These programs are a simple exercise in multiple process programming, multi-threaded programming, and synchronisation.

First we created a simple program that recieves that recieve polinomial operations as input from the user and prints the result.
that is ex3q1.c.

Then we seperated the same idea into 2 distinc processes.
The first recieves the same kind of input from the user, and saves it in a shared memory spaces, that is ex3q2a.c
The second recieves from the shared memory space the polinomial operations, and prints their results on screen, that is ex3q2b.c

Then we added threads.
Cloning ex3q2a.c to ex3q3a.c doing the exact same thing.
Then modified ex3q2b.c to ex3q3b.c, so that addition and subtraction will be done using multiple threads.

# === How to compile ===

simply run make in the directory, a makefile is provided.

# === Input ===

The programs ex3q1, ex3q2a, ex3q3a, will await user input.
Provide a polinomial operation in the following format:
(<Polinomial>)<operation>(<Polinomial>)

Where <operation> must be one of the following:
"ADD" - for addition.
"SUB" - for subtraction.
"MUL" - for multiplication.

and <Polinomial> will represent a polinomial in the following format:
<degree>:<num>,<num>,<num>,...,<num>
Where degree is the degree of the polinomial, and the numbers are the coefficients of the polinomial.
Left to right - largest power to the smallest power.

To stop the process, type END.

Note that if you interrupt the process using control + c, or force kill it from outside in any other way may cause the shared memory space to not be closed.
If that happens, you will need to close the shared memory space manually using ipcrm before you can reopen the program.
Specifically, kill all processes related to this program (meaning anything process from the ones in this folder AND ONLY this folder).
Then type "ipcs -m" in any terminal.
Then search for a process that has 0 mattch.
Then use "ipcrm -m <shmid>" to remove that space.

# === Output ===

ex3q1 will simply output the result on the same terminal.

ex3q2a and ex3q3a will NOT output anything.
For their output, open a seperate terminal and launch ex3q2b or ex3q3b.
