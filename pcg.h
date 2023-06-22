// SPDX-License-Identifier: 0BSD OR MIT-0

typedef struct pcg_random {
	pcg_ulong_t state, inc;
} pcg_t;

/*
 * Initialize a random number generator from the kernel's entropy pool
 */
extern pcg_t pcg_getentropy(void);

/*
 * Properly initialize a random number generator
 * from raw state and (optional) inc values, e.g.
 *
 *	pcg_t rng = pcg_seed((pcg_t){ .state = 0xacab1213 });
 */
extern pcg_t pcg_seed(pcg_t seed);

/*
 * Get a word of random bits from a random number generator
 */
static inline pcg_uint_t pcg_random(pcg_t *rng);

/*
 * Get a random floating point number 0.0 <= ... < 1.0
 */
static inline pcg_fp_t pcg_random_fp(pcg_t *rng);

/* don't call this, call pcg_rand() */
extern pcg_uint_t pcg_rand_slow(
	pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo);

/*
 * Get an unbiased random number less than the given limit
 */
static inline pcg_uint_t
pcg_rand(pcg_t *rng, pcg_uint_t limit) {
	pcg_ulong_t hi_lo = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	if ((pcg_uint_t)(hi_lo) < limit)
		return (pcg_rand_slow(rng, limit, hi_lo));
	return ((pcg_uint_t)(hi_lo >> PCG_UINT_BITS));
}
