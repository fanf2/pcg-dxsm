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

#pragma once

#include "pcg-undef.h"

#define pcg_t            pcg64_t
#define pcg_uint_t       uint64_t
#define pcg_ulong_t      __uint128_t
#define pcg_random       pcg64
#define pcg_seed         pcg64_seed
#define pcg_entropy      pcg64_entropy
#define pcg_uniform      pcg64_uniform
#define pcg_uniform_slow pcg64_uniform_slow

#include "pcg.h"

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
	return(hi);
}
