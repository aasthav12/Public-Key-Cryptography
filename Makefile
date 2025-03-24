CC = clang
CFLAGS = -Wall -Werror -Wextra -Wpedantic $(shell pkg-config --cflags gmp)
LFLAGS = $(shell pkg-config --libs gmp)

all: keygen encrypt decrypt

keygen: keygen.o ss.o randstate.o numtheory.o
	$(CC) -o keygen keygen.o ss.o randstate.o numtheory.o $(LFLAGS) 

encrypt: encrypt.o ss.o randstate.o numtheory.o
	$(CC) -o encrypt encrypt.o ss.o randstate.o numtheory.o $(LFLAGS) 

decrypt: decrypt.o ss.o randstate.o numtheory.o
	$(CC) -o decrypt decrypt.o ss.o randstate.o numtheory.o $(LFLAGS) 
	
keygen.o: keygen.c
	$(CC) $(CFLAGS) -c keygen.c
	
encrypt.o: encrypt.c
	$(CC) $(CFLAGS) -c encrypt.c 
	
decrypt.o: decrypt.c
	$(CC) $(CFLAGS) -c decrypt.c 
	
ss.o: ss.c
	$(CC) $(CFLAGS) -c ss.c
	
randstate.o: randstate.c
	$(CC) $(CFLAGS) -c randstate.c 
	
numtheory.o: numtheory.c
	$(CC) $(CFLAGS) -c numtheory.c 

clean:
	rm -f keygen encrypt decrypt *.o

format:
	clang-format -i -style=file *.[c,h]

