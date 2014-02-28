CC = mpicc

SOURCES = pmerge.c
OBJECTS = $(SOURCES:.c=.o)

CFLAGS = -c -Wall
LDFLAGS = -lm

pmerge: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o pmerge
.c.o:
	$(CC) $(CFLAGS) $< -o $@
