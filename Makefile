.PHONY: all
all: bfi

bf_vector.o: bf_vector.c bf_vector.h
	gcc -Wall -O2 -DNDEBUG -std=c99 -c bf_vector.c

bfi: bf_vector.o bfi.c bfi.h
	gcc -Wall -O2 -DNDEBUG -std=c99 bfi.c bf_vector.o -o bfi

.PHONY: clean
clean:
	rm bfi *.o
