include ../../../_env

PROGS= main sub

# Default Rules:
.c:
	$(CC) $< -o $@

all:    $(PROGS)

main: main.c 
	$(CC)  -c $@.c -o $@.o 

sub: sub.c main.o 
	$(CC) $@.c main.o -o $@ 
	rm -f $@.o

install: all

clean:
	rm -f $(PROGS) *.o
