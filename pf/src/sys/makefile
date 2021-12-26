include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox

PROGS=  pnoxd

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pnoxd: pnoxd.c pxlock.c pxproc.c $(INCLUDE)
	$(CC) $@.c pxlock.c pxproc.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
