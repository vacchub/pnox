include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox

PROGS=  pxmain

# Default Rules:
.c:
	$(CC) $(CFLAGS) $< -o $@.o $(LFLAGS)

all:    $(PROGS)

pxmain: pxmain.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

install: all
	cp $(PROGS).o $(PBIN)

clean:
	rm -f $(PROGS).o *.o
