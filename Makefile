CC=gcc

CFLAG= -Wall

LIBS = 

ifdef DEBUG
CFLAG += -g -DDEBUG
endif

PROG = http-server

all: $(PROG)

$(PROG): generic.h helper.c http.c http-parse.c main.c 
	$(CC) $(CFLAG) $^ -o $@ $(LIBS)

clean:
	rm $(PROG)

# make DEBUG=1