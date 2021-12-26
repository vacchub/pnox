include ../_env

INCLUDE=$(PINC)/pnox.h
CFLAGS= -I$(PINC) 
LFLAGS= -L$(PBIN) -lpnox

PROGS=tpsmtalk mtalk010 v_mbmtalk

TPSBIN= ipcsub mbmtalk mtalk001 mtalk002

TPSOBJ= ipcsub.o mbmtalk.o mtalk001.o mtalk002.o

# Default Rules:
.c:
	$(CC) -c $< -o $@ $(CFLAGS) $(LFLAGS)

all: $(TPSBIN) $(PROGS)

ipcsub: ipcsub.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

mbmtalk: mbmtalk.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

mtalk001: mtalk001.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

mtalk002: mtalk002.c $(INCLUDE)
	$(CC) -c $@.c -o $@.o $(CFLAGS) $(LFLAGS)

tpsmtalk: tpsmtalk.c mtalk001.c mtalk002.c $(INCLUDE)
	$(CC) $@.c $(PBIN)/tpsmain.o $(TPSOBJ) -o $@ $(CFLAGS) $(LFLAGS)

mtalk010: mtalk010.c mbmtalk.h mbmtalk.c $(INCLUDE)
	$(CC) $@.c ipcsub.c mbmtalk.h mbmtalk.c -o $@ $(CFLAGS) $(LFLAGS)

v_mbmtalk: v_mbmtalk.c mbmtalk.h mbmtalk.c $(INCLUDE)
	$(CC) $@.c ipcsub.c mbmtalk.h mbmtalk.c -o $@ $(CFLAGS) $(LFLAGS)

install: all
	chmod 755 $(PROGS)
	cp $(PROGS) $(ABIN)

clean:
	rm -f $(PROGS) *.o
