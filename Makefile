CFLAG=-g 
CC=gcc

LIBS = 


PROG = http_server

all: $(PROG)

http_server: generic.h http.c http_parse.c main.c 
	$(CC) $(CFLAG) -o http_server generic.h http.c http_parse.c main.c $(LIBS)

clean:
	rm $(PROG)