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
 * Serialize the random number generator as a printable string so that
 * the seed can be recorded and the results can be reproduced later.
 *
 * The return value is always the length of the output string excluding the
 * terminating '\0'. If size is too small then nothing is written to buf.
 */
extern size_t pcg_totext(
	char *restrict buf, size_t size, const pcg_t *restrict rng);

/*
 * Deserialize the random number generator from a string, which can be
 * terminated by `\0' or space or any of "\t\n\v\f\r".
 *
 * The return value is zero if there is a syntax error,
 * or the length of the string excluding the terminator.
 */
extern size_t pcg_fromtext(const char *restrict buf, pcg_t *restrict rng);

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
 * Get an unbiased random number less than the limit, using Daniel Lemire's
 * nearly-divisionless rejection sampling algorithm. The fast path is inline;
 * the rarely used re-try loop is in an extern function to save space.
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
 *
 * We treat a value from pcg_random() (which is W = PCG_UINT_BITS wide)
 * as a 0,W bit fixed point value less than 1.0. A double-width multiply
 * by the limit gives us a W,W bit fixed point value less than the limit.
 * The result will be the integer part (upper W bits), and we use the
 * fraction part (lower W bits) to determine if we need to resample.
 *
 * For each upper-half value U, the lower half has a value of the form
 *
 *	L = a + b * limit
 *
 * Values of L are spaced `limit` apart by the multiplication. Depending
 * on U the alignment varies, `0 <= a < limit`, leaving space for `quota`
 * or `quota + 1` possible values for `b` within the range of L. Of the
 * `limit` possible values of U there are `reject` that can be over-quota.
 *
 * We split L's range into an upper span of size `yield` and a lower span
 * of size `reject`. The `yield` span is a multiple of `limit` and always
 * covers exactly `quota` possible values of L, regardless of the
 * alignment; these are the unbiased samples which we return. For some
 * values of U, one over-quota value of L can also fall in the `reject`
 * span; when we get one of these samples we re-try.
 *
 * To avoid the division needed to calculate `reject`, we use `limit`
 * as a safe over-estimate (`limit > range % limit`). The slow path
 * will calculate the exact threshold, re-check and return this sample
 * if it passes, or re-try with another sample.
 */
static inline pcg_uint_t
pcg_rand_fast(pcg_t *rng, pcg_uint_t limit) {
	pcg_ulong_t sample = (pcg_ulong_t) pcg_random_fast(rng) * limit;
	if ((pcg_uint_t)(sample) < limit)
		return (pcg_rand_slow(rng, limit, sample));
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}

/*
 * Get an unbiased random number less than the limit, where the limit is a
 * constant greater than zero, so the compiler can optimize out the division.
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
	pcg_ulong_t sample;
	do sample = (pcg_ulong_t) pcg_random_fast(rng) * limit;
	while ((pcg_uint_t)(sample) < reject);
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
