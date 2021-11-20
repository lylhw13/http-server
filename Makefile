CFLAG=-g 
CC=gcc

LIBS = 


PROG = http-server

all: $(PROG)

$(PROG): generic.h helper.c http.c http-parse.c main.c 
	$(CC) $(CFLAG) $^ -o $@ $(LIBS)

clean:
	rm $(PROG)