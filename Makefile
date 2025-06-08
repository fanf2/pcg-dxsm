# SPDX-License-Identifier: 0BSD OR MIT-0

.POSIX:
.PHONY: all clean format
.SUFFIXES: .c .h .o .def

CFLAGS= -O2 -Wall -Wextra -D_DEFAULT_SOURCE
LDFLAGS= ${CFLAGS} -lm

all: test
	./test

clean:
	rm -f pcg32.[cho] pcg64.[cho]
	rm -f test bytes bytes-* bytes.[ho]
	rm -f floats floats.o fcvt.o

format:
	clang-format -i *.[ch]

test:    test.o pcg32.o pcg64.o
test.o:  test.c pcg32.h pcg64.h

bytes:   bytes.o pcg32.o
bytes.o: bytes.c bytes.h nanotime.h pcg32.h

floats: floats.o fcvt.o pcg32.o
floats.o: floats.c nanotime.h pcg32.h

pcg32.o: pcg32.c pcg32.h
pcg64.o: pcg64.c pcg64.h
pcg32.c: pcg32.def pcg.c pcg_blurb.c
pcg64.c: pcg64.def pcg.c pcg_blurb.c
pcg32.h: pcg32.def pcg.h pcg_blurb.h pcg32_xsh_rr.c
pcg64.h: pcg64.def pcg.h pcg_blurb.h pcg64_dxsm.c

.def.c:
	sed 's/pcg/$*/g;/^[ /]\*/d' pcg_blurb.c >$@
	cat $*.def pcg.c |\
	${CC} -E - | sed '/^#/d;/^$$/d' | clang-format >>$@

.def.h:
	sed 's/pcg/$*/g;/^[ /]\*/d' pcg_blurb.h >$@
	cat $*.def pcg.h $*_*.c |\
	${CC} -E - | sed '/^#/d;/^$$/d' | clang-format >>$@

bytes.h: bytes-gen
	./bytes-gen
	${CC} -E bytes-vec.h | sed '/^#/d;/^$$/d' | clang-format |\
	cat bytes-mul.h - >bytes.h

bytes-gen: bytes.c
	${CC} ${CFLAGS} -DGENERATE -o bytes-gen bytes.c

fcvt.o: floats.c
	${CC} ${CFLAGS} -DSEPARATE -c -o fcvt.o floats.c
