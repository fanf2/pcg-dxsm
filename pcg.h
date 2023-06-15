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

/*
 * No #pragma once because this header depends on the macros defined
 * in "pcg32.h" and "pcg64.h" and you can use both in the same file.
 */

#include <stdint.h>

typedef struct pcg_random {
	pcg_ulong_t state, inc;
} pcg_t;

/*
 * Initialize a random number generator from the kernel's entropy pool
 */
extern pcg_t
pcg_entropy(void);

/*
 * Seed a random number generator from raw state and inc values.
 */
extern pcg_t
pcg_seed(pcg_t seed);

/*
 * Get a word of random bits from a random number generator
 */
static inline pcg_uint_t
pcg_random(pcg_t *rng);

/* don't call this, call pcg_uniform() */
extern pcg_uint_t
pcg_uniform_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo);

#define PCG_UINT_BITS (sizeof(pcg_uint_t) * 8)

/*
 * Get an unbiased random number less than the given limit
 */
static inline pcg_uint_t
pcg_uniform(pcg_t *rng, pcg_uint_t limit) {
	pcg_ulong_t hi_lo = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	if ((pcg_uint_t)(hi_lo) < limit)
		return (pcg_uniform_slow(rng, limit, hi_lo));
	return ((pcg_uint_t)(hi_lo >> PCG_UINT_BITS));
}
