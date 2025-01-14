#!/bin/bash

compile:
	gcc -c primary_storage.c -o primary_storage.o
	gcc -c user_interface.c -o user_interface.o -lncurses
	gcc primary_storage.o user_interface.o -o spreadsheet -lncurses
	rm *.o

aks:
	gcc -c primary_storage.c -o primary_storage.o
	gcc -c main.c -o main.o
	gcc primary_storage.o user_interface.o -o aks
	rm *.o

run:
	./spreadsheet