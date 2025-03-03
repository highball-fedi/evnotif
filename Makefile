CC = cc
CFLAGS = -std=c99
LDFLAGS = -std=c99
LIBS =

.PHONY: all clean
.SUFFIXES: .c .o

all: evnotif

evnotif: evnotif.o
	$(CC) $(LDFLAGS) -o $@ evnotif.o $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o evnotif
