#!/bin/bash

spreadsheet: primary_storage.o compute_unit.o user_interface.o
	gcc primary_storage.c compute_unit.c user_interface.c -o spreadsheet -lncurses -lm
	rm *.o

primary_storage.o: primary_storage.c
	gcc -c primary_storage.c -o primary_storage.o

compute_unit.o: compute_unit.c
	gcc -c compute_unit.c -o compute_unit.o

user_interface.o: user_interface.c
	gcc -c user_interface.c -o user_interface.o -lncurses -lm


aks: main.o primary_storage.o compute_unit.o
	gcc primary_storage.o main.o compute_unit.o -o aks -lm
	rm *.o

main.o: main.c