// SPDX-License-Identifier: 0BSD OR MIT-0

typedef struct pcg {
	pcg_ulong_t state, inc;
} pcg_t;

/*
 * Initialize a random number generator from the kernel's entropy pool
 */
extern pcg_t pcg_getentropy(void);

/*
 * Initialize a random number generator to a fixed sequence
 * based on the given state and (optional) inc values, e.g.
 *
 *	pcg_t rng = pcg_seed((pcg_t){ .state = 3141592654 });
 */
extern pcg_t pcg_seed(pcg_t seed);

/*
 * Get a word full of random bits
 *
 * The pcg_random() macro automatically selects a suitable alternative
 */
static inline pcg_uint_t pcg_random_fast(pcg_t *rng);
extern pcg_uint_t pcg_random_small(pcg_t *rng);

/*
 * Get an unbiased random integer, 0 <= ... < limit
 *
 * The pcg_rand() macro automatically selects a suitable alternative
 */
static inline pcg_uint_t pcg_rand_fast(pcg_t *rng, pcg_uint_t limit);
extern pcg_uint_t pcg_rand_small(pcg_t *rng, pcg_uint_t limit);
/*
 * Internal helper out-of-line slow path for pcg_rand_fast()
 */
extern pcg_uint_t pcg_rand_slow(
	pcg_t *rng, pcg_uint_t limit, pcg_ulong_t sample);

/*
 * Get a random floating point number, 0.0 <= ... < 1.0
 */
static inline pcg_fp_t pcg_fp(pcg_t *rng);

/*
 * Write `size` random bytes at `ptr`
 */
extern void pcg_bytes(pcg_t *restrict rng, void *restrict ptr, size_t size);

/*
 * Shuffle the array at `ptr` containing `count` objects of size `size`
 */
extern void pcg_shuffle(
	pcg_t *restrict rng, void *restrict ptr, pcg_uint_t count, size_t size);

/*
 * Internal helper to get a biased random number logically less than the
 * limit. Treat a value from pcg_random() (which is W = PCG_UINT_BITS
 * wide) as a 0,W bit fixed point value less than 1.0. A double width
 * multiply by the limit gives us a W,W bit fixed point value less than
 * the limit. The caller will extract the result from the integer part
 * (upper W bits), and use the fraction part (lower W bits) to determine
 * if it needs to resample, as explained below.
 */
static inline pcg_ulong_t
pcg_ulong_biased(pcg_t *rng, pcg_uint_t limit) {
	return((pcg_ulong_t)pcg_random_fast(rng) * (pcg_ulong_t)limit);
}

/*
 * Get an unbiased random number less than the limit, where the limit is
 * a constant greater than zero. We use Daniel Lemire's rejection sampling
 * algorithm, tuned for calculating the reject threshold at compile time.
 *
 *	range = 1 << PCG_UINT_BITS
 *	reject = range % limit
 *	yield = range - reject
 *	quota = yield / limit
 *
 * The number of possible `sample` values is the same as the `range` of
 * possible values returned by pcg_random(). We will return a result if
 * the `sample` is one of `yield` possible values, where `yield` is the
 * largest multiple of `limit` less than `range`. (The largest multiple
 * makes resampling as rare as possible.) We ensure our results will be
 * unbiased by mapping `quota` of the possible sample values to each of
 * the `limit` possible return values. When the `sample` is over-quota,
 * it is one of the `reject` possible values that cause a re-try.
 */
static inline pcg_uint_t
pcg_rand_const(pcg_t *rng, pcg_uint_t limit) {
	/*
	 * The value of `range` doesn't fit into a `pcg_uint_t`, so we use
	 * a trick to calculate `reject` without overflow:
	 *
	 * reject =                range % limit
	 *       ==      (range - limit) % limit // equiv modulo limit
	 *       == (pcg_uint_t)(-limit) % limit // equiv modulo range
	 *
	 * This % is safe because of the guard in the pcg_rand() macro.
	 */
	pcg_uint_t reject = -limit % limit;
	/*
	 * The upper half of `sample` has `limit` possible values; for each
	 * upper-half value U, the lower half has a value of the form
	 *
	 *	L = a + b * limit
	 *
	 * Lower-half values are spaced `limit` apart by the multiplication.
	 * Depending on U, their alignment varies, `0 <= a < limit`, leaving
	 * `quota` or `quota + 1` possible values for `b`, and therefore L.
	 *
	 * We split the `range` covering L into two spans of size `yield`
	 * and `reject`. The `yield` span is a multiple of `limit` so it
	 * always covers exactly `quota` possible values of L, regardless
	 * of the alignment; we return these unbiased samples. For some
	 * values of U, one over-quota value of L can also fall in the
	 * `reject` span; when we get one of these samples we re-try.
	 */
	pcg_ulong_t sample;
	do sample = pcg_ulong_biased(rng, limit);
	while ((pcg_uint_t)(sample) < reject);
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}

/*
 * Get an unbiased random number less than the limit, where the limit is
 * unknown until run time. Daniel Lemire's nearly-divisionless algorithm
 * has the same rejection sampling loop as above, but avoids calculating
 * the reject threshold in most cases. The fast path is inlined; the
 * rarely used rejection loop is in an extern function to save space.
 */
static inline pcg_uint_t
pcg_rand_fast(pcg_t *rng, pcg_uint_t limit) {
	/*
	 * Get a sample and quickly check if it is unbiased using `limit`
	 * as a safe over-estimate for the reject threshold (`limit` is
	 * greater than `anything % limit`). The slow path will calculate
	 * the exact threshold as above, re-check and return this sample
	 * if it passes, or re-try with another sample.
	 */
	pcg_ulong_t sample = pcg_ulong_biased(rng, limit);
	if ((pcg_uint_t)(sample) < limit)
		return (pcg_rand_slow(rng, limit, sample));
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
