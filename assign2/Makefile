CC = gcc
SOURCES = distsum.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -c -Wall

distsum: $(OBJECTS)
	$(CC) $(OBJECTS) -o distsum
.c.o:
	$(CC) $(CFLAGS) $< -o $@