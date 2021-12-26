include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) -lpnox

PROGS=omok0001

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ $(PBIN)/pxmain.o

all:    $(PROGS)

omok0001: omok0001.c $(INCLUDE) $(PBIN)/pxmain.o
	$(CC) $@.c -o $@ -D$@=pnoxsvc $(PBIN)/pxmain.o $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(ABIN)

clean:
	rm -f $(PROGS) *.o
