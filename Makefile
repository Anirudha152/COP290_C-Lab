#!/bin/bash

compile:
	gcc -c primary_storage.c -o primary_storage.o
	gcc -c user_interface.c -o user_interface.o -lcurses
	gcc primary_storage.o user_interface.o -o spreadsheet -lcurses
	rm *.o

run:
	./spreadsheet