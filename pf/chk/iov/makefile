include ../../src/_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox


# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

install: all

clean:
	rm -f $(PROGS) *.o
