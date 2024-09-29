// SPDX-License-Identifier: 0BSD OR MIT-0

typedef struct pcg {
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
static inline pcg_fp_t pcg_fp(pcg_t *rng);

/*
 * Write `size` random bytes at `ptr`
 */
extern void pcg_bytes(pcg_t *restrict rng, void *restrict ptr, size_t size);

/*
 * Get an unbiased random number less than the given limit
 */
extern pcg_uint_t pcg_rand(pcg_t *rng, pcg_uint_t limit);

/*
 * The macro pcg_rand_fast(rng, limit) defined in "pcg_blurb.h" is
 * like pcg_rand(rng, limit) but the fast path is inlined.
 *
 * Don't call pcg_rand_inline() directly, call pcg_rand_fast().
 */
static inline pcg_uint_t
pcg_rand_inline(pcg_t *rng, pcg_uint_t limit, int maybe_slow) {
	extern pcg_uint_t pcg_rand_slow(
		pcg_t * rng, pcg_uint_t limit, pcg_ulong_t sample);
	/*
	 * Daniel Lemire's nearly-divisionless unbiased bounded random numbers.
	 *
	 * We get a value W = PCG_UINT_BITS wide from pcg_random(). We can
	 * think of it as a 0.W bit fixed-point value less than 1.0. When
	 * we do a double-width multiply by the limit, we get a W.W bit
	 * fixed-point value less than the limit. Our result will be the
	 * integer part (upper W bits), and we will use the fraction part
	 * (lower W bits) to determine whether or not we need to resample.
	 */
	pcg_ulong_t sample = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	/*
	 * The compile-time value maybe_slow is false when the integer part
	 * of the sample is trivially unbiased. The slow path will calculate
	 * the resample threshold using `% limit`; we can avoid the `%` by
	 * using `limit` as a slight over-estimate of the exact threshold.
	 */
	if (maybe_slow && (pcg_uint_t)(sample) < limit)
		return (pcg_rand_slow(rng, limit, sample));
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
