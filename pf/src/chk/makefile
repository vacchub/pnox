include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox -lpthread

PROGS= 

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

install: all
#	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
