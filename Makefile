# the compiler: gcc for C program, define as g++ for C++
CC = gcc

# compiler flags:
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
CFLAGS  = -g -Wall -Wextra -O2 -Wno-unknown-pragmas

.PHONY: clean all

all: clean src/SoE_seq src/SoE_omp src/SoE_omp_block

src/SoE_seq: build src/bitter.o src/main.c
	$(CC) $(CFLAGS) src/bitter.o src/main.c -lm -lpapi -o build/SoE_seq

src/SoE_omp: build src/bitter.o src/main.c
	$(CC) $(CFLAGS) src/bitter.o src/main.c -lm -DOMP -fopenmp -lpapi -o build/SoE_omp

src/SoE_omp_block: build src/block_decomposition.c src/bitter.o
	$(CC) $(CFLAGS) src/block_decomposition.c src/bitter.o -lm -DOMP -fopenmp -lpapi -o build/SoE_omp_block

src/bitter.o: src/bitter.c
	$(CC) $(CFLAGS) -c src/bitter.c -o src/bitter.o 

build:
	mkdir build
clean:
	rm -rf build