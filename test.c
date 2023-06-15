/*
 * Written by Tony Finch <dot@dotat.at> in Cambridge.
 *
 * Permission is hereby granted to use, copy, modify, and/or
 * distribute this software for any purpose with or without fee.
 *
 * This software is provided 'as is', without warranty of any kind.
 * In no event shall the authors be liable for any damages arising
 * from the use of this software.
 *
 * SPDX-License-Identifier: 0BSD OR MIT-0
 */

#include <inttypes.h>
#include <stdio.h>

#include "pcg32.h"
#include "pcg64.h"

int
main(void) {
	pcg32_t rng32 = pcg32_entropy();
	pcg64_t rng64 = pcg64_entropy();
	printf("%" PRIu32 "\n", pcg32_uniform(&rng32, INT32_MAX));
	printf("%" PRIu64 "\n", pcg64_uniform(&rng64, INT64_MAX));
}
