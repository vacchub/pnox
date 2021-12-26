include ../src/_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) -I$(AINC)
LFLAGS= -L$(PBIN) -lpnox

PROGS=

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $(INCLUDE) $< -o $@ 

all:    $(PROGS)

install: all

clean:
	rm -f $(PROGS) *.o
