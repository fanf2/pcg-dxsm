CFLAGS= -O2 -Weverything -Wno-poison-system-directories

all: pcg64.o pcg32.o

clean:
	rm -f pcg64.o pcg32.o

pcg32.o: pcg32.c pcg32.h pcg.c pcg.h pcg-undef.h

pcg64.o: pcg64.c pcg64.h pcg.c pcg.h pcg-undef.h
