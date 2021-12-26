include ../../../_env

INCLUDE=$(PINC)/pnox.h $(PINC)/stream.h
CFLAGS= -I$(PINC) -D_THREAD_SAFE
LFLAGS= -L$(PLIB) $(L_BASE) -lpnox -lsocket -lpthread

PROGS=mfork

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ 

all:    $(PROGS)

mfork: mfork.c
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all

clean:
	rm -f $(PROGS) *.o
