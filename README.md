vm-emulator
===========

This repository contains a virtual machine emulator which was written in C for a school assignment. I received a grade of 20/20 on this assignment.

The emulator, compiled from *vm.c* as described below, will interpret and execute assembly code written in a simplified subset of the x86 assembly language. The repository contains four sample input files written in this assembly subset: *fact.vm*, *fib.vm*, *jsr.vm*, and *loop.vm*.

The source code is in *vm.c* and can be compiled simply with `gcc -Wall -Wextra vm.c -o vm`. The usage of the compiled executable is `./vm inputfile.vm`.

In addition to interpreting and executing the input assembly code, the emulator will also gather profiling statistics describing the basic blocks of the program represented by the assembly code, and the frequency with which each basic block is executed. Profiling statistics will be written to *profile.txt*, which will be overwritten if it already exists.
