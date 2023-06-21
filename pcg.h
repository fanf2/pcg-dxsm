// SPDX-License-Identifier: 0BSD OR MIT-0

typedef struct pcg_random {
	pcg_ulong_t state, inc;
} pcg_t;

/*
 * Initialize a random number generator from the kernel's entropy pool
 */
extern pcg_t pcg_entropy(void);

/*
 * Seed a random number generator from raw state and inc values.
 */
extern pcg_t pcg_seed(pcg_t seed);

/*
 * Get a word of random bits from a random number generator
 */
static inline pcg_uint_t pcg_random(pcg_t *rng);

/* don't call this, call pcg_uniform() */
extern pcg_uint_t pcg_uniform_slow(
	pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo);

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
