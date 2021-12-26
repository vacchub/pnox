include ../_env

INCLUDE=$(PINC)/pnox.h
SHLLSO= libpnox.so
SHLLIB= libpnox.a
SHLLSL= libpnox.sl
CFLAGS= $(C_BASE) -I$(PINC) -D_REENTRANT
LFLAGS=

SHLOBJ= pxopen.o	\
	pxexec.o	\
	pxenv.o		\
	pxfunc.o	\
	pxmon.o		\
	syslog.o	\
	ipcsub.o	\
	svrsnd.o	\
	string.o	\
	pxsndfd.o	\
	pxrcvfd.o

# Default Rules:
.c:
	$(CC) -c $< -o $@ $(CFLAGS) $(LFLAGS) 
all:    $(SHLOBJ)


install: all $(TARGET).install

LINUX.install:
	$(CC) -shared -o $(SHLLSO) $(SHLOBJ)
	cp $(SHLLSO) $(PBIN)
SUNOS.install:
	ld -G -o $(SHLLSO) $(SHLOBJ)
	cp $(SHLLSO) $(PBIN)
AIX.install:
	ld -o $(SHLLIB) $(SHLOBJ) -bE:libpnox.exp -bM:SRE -lc -lm
	cp $(SHLLIB) $(PBIN)
HPUX10.install:
	ld -b -o $(SHLLSL) $(SHLOBJ)
	cp $(SHLLSL) $(PBIN)
HPUX11.install:
	ld -b -o $(SHLLSL) $(SHLOBJ)
	cp $(SHLLSL) $(PBIN)
clean:
	rm -f *.o
