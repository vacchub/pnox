include ../_env

CFLAGS=$(C_BASE) 
LFLAGS=$(L_BASE) $(L_THRD)

PROGS=	pxrush

# Default Rules:
.c:
	$(CC) $(CFLAGS) $(LFLAGS) $< -o $@

all:    $(PROGS)

pxrush: pxrush.c agent.c pxrush.h
	$(CC) $@.c agent.c -o $@ $(CFLAGS) $(LFLAGS)
	rm -f $@.o

install: all
	cp $(PROGS) $(PBIN)

clean:
	rm -f $(PROGS) *.o
