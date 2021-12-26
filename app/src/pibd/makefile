include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PBIN) -lpnox

PROGS=pibd6001 pibd6002 pibd6003

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@ 

all:    $(PROGS)

pibd6001: pibd6001.c subf6001.c subf6001.h $(INCLUDE)
	$(CC) $@.c subf6001.c subf6001.h -o $@ $(CFLAGS) $(LFLAGS) -lcurl -ljansson
	rm -f $@.o

pibd6002: pibd6002.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

pibd6003: pibd6003.c $(INCLUDE)
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(ABIN)

clean:
	rm -f $(PROGS) *.o
