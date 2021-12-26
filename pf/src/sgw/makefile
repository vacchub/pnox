include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox

PROGS=  pnoxsgw

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pnoxsgw: pnoxsgw.c initial.c exesca.c exerta.c $(INCLUDE)
	$(CC) $@.c initial.c exesca.c exerta.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
