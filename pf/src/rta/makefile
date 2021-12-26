include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox

PROGS=  pnoxrta

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pnoxrta: pnoxrta.c service.c $(INCLUDE)
	$(CC) $@.c service.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
