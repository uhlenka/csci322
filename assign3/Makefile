CC = mpicc

CENTSOURCES = central.c
CENTOBJECTS = $(CENTSOURCES:.c=.o)

SYMSOURCES = symmetric.c
SYMOBJECTS = $(SYMSOURCES:.c=.o)

RINGSOURCES = ring.c
RINGOBJECTS = $(RINGSOURCES:.c=.o)

CFLAGS = -c -Wall
LDFLAGS = -lm

central: $(CENTOBJECTS)
	$(CC) $(CENTOBJECTS) $(LDFLAGS) -o central
symmetric: $(SYMOBJECTS)
	$(CC) $(SYMOBJECTS) $(LDFLAGS) -o symmetric
ring: $(RINGOBJECTS)
	$(CC) $(RINGOBJECTS) $(LDFLAGS) -o ring
.c.o:
	$(CC) $(CFLAGS) $< -o $@
