#!/bin/sh

EXE="vm"
valgrind --leak-check=full ./$EXE fact.vm > /dev/null
# gdb ./$EXE
