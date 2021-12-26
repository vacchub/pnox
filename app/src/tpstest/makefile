include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) -lpnox

PROGS=tpstest

TPSBIN= subx0001

TPSOBJ= subx0001.o

# Default Rules:
.c:
	$(CC) -c $< -o $@ $(CFLAGS) $(LFLAGS)

all: $(TPSBIN) $(PROGS)

subx0001: subx0001.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

tpstest: tpstest.c $(INCLUDE)
	$(CC) $@.c $(PBIN)/tpsmain.o $(TPSOBJ) -o $@ $(CFLAGS) $(LFLAGS)

install: all
	chmod 755 $(PROGS)
	cp $(PROGS) $(ABIN)

clean:
	rm -f $(PROGS) *.o
