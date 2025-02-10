# SPDX-License-Identifier: 0BSD OR MIT-0

.POSIX:
.PHONY: all clean format
.SUFFIXES: .c .h .o .def

CFLAGS= -Os -Wall -Wextra

all: test
	./test

clean:
	rm -f pcg32.? pcg64.? test

format:
	clang-format -i *.[ch]

test: test.c pcg32.o pcg64.o

pcg32.o: pcg32.c pcg32.h
pcg64.o: pcg64.c pcg64.h
pcg32.c: pcg32.def pcg.c pcg_blurb.c
pcg64.c: pcg64.def pcg.c pcg_blurb.c
pcg32.h: pcg32.def pcg.h pcg_blurb.h pcg32_xsh_rr.c
pcg64.h: pcg64.def pcg.h pcg_blurb.h pcg64_dxsm.c

.def.c:
	sed 's/pcg/$*/g;/^[ /]\*/d' pcg_blurb.c >$@
	cat $*.def pcg.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@

.def.h:
	sed 's/pcg/$*/g;/^[ /]\*/d' pcg_blurb.h >$@
	cat $*.def pcg.h $*_*.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@
