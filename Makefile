CFLAG=-g 
CC=gcc

LIBS = 


PROG = http_server

all: $(PROG)

http_server: main.c 
	$(CC) $(CFLAG) -o http_server generic.h main.c $(LIBS)

clean:
	rm $(PROG) *.o *.out