# Charles Drews (csd305@nyu.edu, N11539474)
# Homework 1
# Virtual Machines: Concepts and Applications , CSCI-GA.3033-015, Spring 2014

CC=gcc
CFLAGS=-Wall -Wextra -ggdb
SOURCES=vm.c
EXE=vm

all: $(SOURCES) $(EXE)
	$(CC) $(CFLAGS) $(SOURCES) -o $(EXE)
