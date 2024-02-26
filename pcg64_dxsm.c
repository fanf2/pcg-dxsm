// SPDX-License-Identifier: 0BSD OR MIT-0

static inline uint64_t
pcg64_random(pcg64_t *rng) {
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

static inline double
pcg64_random_double(pcg64_t *rng) {
	return ((double)(pcg64_random(rng) >> 11) * 0x1.0p-53);
}
