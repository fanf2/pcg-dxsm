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

#define pcg_t            pcg32_t
#define pcg_uint_t       uint32_t
#define pcg_ulong_t      uint64_t
#define pcg_random       pcg32
#define pcg_seed         pcg32_seed
#define pcg_entropy      pcg32_entropy
#define pcg_uniform      pcg32_uniform
#define pcg_uniform_slow pcg32_uniform_slow

#include "pcg.h"

static inline uint32_t
pcg32(pcg32_t *rng) {
	/* linear congruential generator */
	uint64_t state = rng->state;
	rng->state = state * 6364136223846793005ULL + rng->inc;
	/* XSH RR (xor shift rotate right) permuted output */
	uint32_t xor = (uint32_t)(((state >> 18) ^ state) >> 27);
	uint32_t rot = (uint32_t)(state >> 59);
	return((xor >> (+rot & 31)) | (xor << (-rot & 31)));
}
