CC=gcc
CFLAGS=-Wall -O2 -DNDEBUG -std=c99

.PHONY: all
all: bfi

ast.o: ast.c ast.h
	gcc $(CFLAGS) -c $<

bf_vector.o: bf_vector.c bf_vector.h
	gcc $(CFLAGS) -c $<

logging.o: ast.o logging.c logging.h
	gcc $(CFLAGS) -c logging.c

parser.o: ast.o bf_vector.o parser.c parser.h
	gcc $(CFLAGS) -c parser.c

passes.o: ast.o bf_vector.o logging.o passes.c passes.h
	gcc $(CFLAGS) -c passes.c

file_reader.o: file_reader.c file_reader.h
	gcc $(CFLAGS) -c file_reader.c

bfi: ast.o bf_vector.o logging.o parser.o passes.o file_reader.o bfi.c bfi.h
	gcc $(CFLAGS) bfi.c bf_vector.o ast.o logging.o parser.o passes.o file_reader.o -o bfi

.PHONY: clean
clean:
	rm bfi *.o
