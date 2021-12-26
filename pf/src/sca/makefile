include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE) -D_REENTRANT
LFLAGS= -L$(PBIN) $(L_BASE) -lpthread -lpnox

PROGS=  pnoxsca

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pnoxsca: pnoxsca.c service.c $(INCLUDE)
	$(CC) $@.c service.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
