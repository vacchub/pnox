include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) -lpnox

PROGS=pxlogoff

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ $(PBIN)/pxmain.o

all:    $(PROGS)

pxlogoff: pxlogoff.c ../tpsmtalk/mbmtalk.h ../tpsmtalk/mbmtalk.c $(INCLUDE) $(PBIN)/pxmain.o
	$(CC) $@.c ../tpsmtalk/ipcsub.c ../tpsmtalk/mbmtalk.h ../tpsmtalk/mbmtalk.c -o $@ -D$@=pnoxsvc $(PBIN)/pxmain.o $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(ABIN)

clean:
	rm -f $(PROGS) *.o
