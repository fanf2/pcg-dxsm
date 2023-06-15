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

static inline uint64_t
pcg64(pcg64_t *rng) {
	/* cheap (half-width) multiplier */
	const uint64_t mul = 15750249268501108917ULL;
	/* linear congruential generator */
	pcg_ulong_t state = rng->state;
	rng->state = state * mul + rng->inc;
	/* DXSM (double xor shift multiply) permuted output */
	uint64_t hi = (uint64_t)(state >> 64);
	uint64_t lo = (uint64_t)(state | 1);
	hi ^= hi >> 32;
	hi *= mul;
	hi ^= hi >> 48;
	hi *= lo;
	return (hi);
}
