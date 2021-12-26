include ../../../_env

INCLUDE=$(PINC)/pnox.h $(PINC)/path.h
CFLAGS= -I$(PINC)
LFLAGS= -L$(PLIB) $(L_BASE) -lpnox

PROGS= ccc a b

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

C_SRC= a.c b.c
C_OBJS=$(C_SRC:.c=.o)

all:    $(PROGS)

ccc: $(C_SRC) $(C_OBJS)
	$(MPSBLD) -o ccc $(CFLAGS) $(LFLAGS) $(C_OBJS)

a: a.c $(INCLUDE) 
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

b: b.c $(INCLUDE) 
	$(CC) $@.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
