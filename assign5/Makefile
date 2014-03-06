CC = gcc

SEMSOURCES = atomic_sem.c
SEMOBJECTS = $(SEMSOURCES:.c=.o)

CONDSOURCES = atomic_cond.c
CONDOBJECTS = $(CONDSOURCES:.c=.o)

CFLAGS = -c -g -Wall
LDFLAGS = -lm
LIBFLAGS = -lpthread

atomic_sem: $(SEMOBJECTS)
	$(CC) $(SEMOBJECTS) $(LDFLAGS) -o atomic_sem $(LIBFLAGS)
atomic_cond: $(CONDOBJECTS)
	$(CC) $(CONDOBJECTS) $(LDFLAGS) -o atomic_cond $(LIBFLAGS)
.c.o:
	$(CC) $(CFLAGS) $< -o $@
