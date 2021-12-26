include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) $(C_BASE)
LFLAGS= -L$(PBIN) $(L_BASE) -lpnox

PROGS=  pxinit pxver pxmon pxusleep pxlogout pnoxpoll pxtps pxrun
#PROGS=  pxinit pxver pxmon pxusleep pxlogoff pnoxpoll pxtps pxrun

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pxinit: pxinit.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxver: pxver.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxmon: pxmon.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxusleep: pxusleep.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxlogout: pxlogout.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pnoxpoll: pnoxpoll.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxtps: pxtps.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pxrun: pxrun.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
