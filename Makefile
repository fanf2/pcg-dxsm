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
	rm -f pcg64.[cho] pcg32.[cho] test

test: test.c pcg32.o pcg64.o

pcg32.o: pcg32.c pcg32.h

pcg64.o: pcg64.c pcg64.h

pcg32.h: .cpp .clang-format pcg32_define.h pcg.h pcg32_xsh_rr.h
	cat LICENCE .line .once .line >pcg32.h
	./.cpp pcg32_define.h pcg.h pcg32_xsh_rr.h >>pcg32.h

pcg64.h: .cpp .clang-format pcg64_define.h pcg.h pcg64_dxsm.h
	cat LICENCE .line .once .line  >pcg64.h
	./.cpp pcg64_define.h pcg.h pcg64_dxsm.h >>pcg64.h

pcg32.c: .cpp .clang-format pcg32_define.h .pcg32.h pcg.c
	cat LICENCE .line pcg_include.c .line .pcg32.h .line >pcg32.c
	./.cpp pcg32_define.h pcg.c >>pcg32.c

pcg64.c: .cpp .clang-format pcg64_define.h .pcg64.h pcg.c
	cat LICENCE .line pcg_include.c .line .pcg64.h .line >pcg64.c
	./.cpp pcg64_define.h pcg.c >>pcg64.c

format:
	clang-format -i *.[ch]
