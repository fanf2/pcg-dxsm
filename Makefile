# SPDX-License-Identifier: 0BSD OR MIT-0

.SUFFIXES: .c .h .o .def

CFLAGS= -O2 -Wall -Wextra

all: test

clean:
	rm -f *.o test

dirty:
	rm -f pcg32.? pcg64.?

format:
	clang-format -i *.[ch]

# normal build

test: test.c pcg32.o pcg64.o

pcg32.o: pcg32.c pcg32.h
pcg64.o: pcg64.c pcg64.h
pcg32.c: pcg32.def pcg.c pcg_blurb.c
pcg64.c: pcg64.def pcg.c pcg_blurb.c
pcg32.h: pcg32.def pcg.h pcg_blurb.h pcg32_xsh_rr.c
pcg64.h: pcg64.def pcg.h pcg_blurb.h pcg64_dxsm.c

.def.c:
	cat pcg_blurb.c >$@
	printf '#include "$*.h"\n\n' >>$@
	cat $*.def pcg.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@

.def.h:
	cat pcg_blurb.h >$@
	cat $*.def pcg.h $*_*.c |\
	cc -E - | sed '/^#/d;/^$$/d' | clang-format >>$@
