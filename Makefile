CC=gcc
CFLAGS=-Wall

all: main assembler
main:
	gcc -o interpreter src/*.c main.c
assembler:
	gcc -o assembler assembler.c

