vm-emulator
===========

This repository contains a virtual machine emulator which was written in C for a school assignment. I received a grade of 20/20 on this assignment.

The emulator, compiled from *vm.c* as described below, will interpret and execute assembly code written in a simplified subset of the x86 assembly language. The repository contains four sample input files written in this assembly subset: *fact.vm*, *fib.vm*, *jsr.vm*, and *loop.vm*.

The intent of the project is to demonstrate how an emulator can be used to execute assembly code rather than executing the assembly code natively. There are two main reasons a user would want to use an emulator:
  1. The assembly code is in an assembly language that differs from the user's platform (i.e. if this emulator were compiled and run on an ARM platform, it could be used to execute the sample x86 assembly code input files which could not be executed natively on an ARM platform)
  2. The user wishes gather profiling statistics describing the execution of the input code

The source code of my emulator is in *vm.c* and can be compiled simply with `gcc -Wall -Wextra vm.c -o vm`. The usage of the compiled executable is `./vm inputfile.vm`.

In addition to interpreting and executing the input assembly code, the emulator will also gather profiling statistics describing the basic blocks of the program represented by the assembly code, and the frequency with which each basic block is executed. Profiling statistics will be written to *profile.txt*, which will be overwritten if it already exists.
