# SPDX-License-Identifier: 0BSD OR MIT-0

.POSIX:
.PHONY: all test clean dirty format
.SUFFIXES: .c .h .o .def

CFLAGS= -Os -Wall -Wextra
ARFLAGS= -rcs

OBJ = pcg32.o pcg64.o shuffle.o

all: test
	./test

test: test.c pcg6432.a

clean:
	rm -f ${OBJ} pcg6432.a test

dirty:
	rm -f pcg32.? pcg64.?

format:
	clang-format -i *.[ch]

pcg6432.a: ${OBJ}
	${AR} ${ARFLAGS} $@ ${OBJ}

pcg32.o: pcg32.c pcg32.h
pcg64.o: pcg64.c pcg64.h
pcg32.c: pcg32.def pcg.c pcg_blurb.c
pcg64.c: pcg64.def pcg.c pcg_blurb.c
pcg32.h: pcg32.def pcg.h pcg_blurb.h pcg32_xsh_rr.c
pcg64.h: pcg64.def pcg.h pcg_blurb.h pcg64_dxsm.c
shuffle.o: shuffle.c pcg32.h pcg64.h

.def.c:
	sed '/^\/\*/d' pcg_blurb.c >$@
	printf '#include "$*.h"\n\n' >>$@
	cat $*.def pcg.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@

.def.h:
	sed 's/pcg/$*/g;/^[ /]\*/d' pcg_blurb.h >$@
	cat $*.def pcg.h $*_*.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@
