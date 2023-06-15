# Written by Tony Finch <dot@dotat.at> in Cambridge.
#
# Permission is hereby granted to use, copy, modify, and/or
# distribute this software for any purpose with or without fee.
#
# This software is provided 'as is', without warranty of any kind.
# In no event shall the authors be liable for any damages arising
# from the use of this software.
#
# SPDX-License-Identifier: 0BSD OR MIT-0

CFLAGS= -O2 -Weverything -Wno-poison-system-directories

all: test

clean:
	rm -f pcg64.o pcg32.o test

test: test.c pcg32.o pcg64.o

pcg32.o: pcg32.c pcg32.h pcg.c pcg.h pcg-undef.h

pcg64.o: pcg64.c pcg64.h pcg.c pcg.h pcg-undef.h
