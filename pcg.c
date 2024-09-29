// SPDX-License-Identifier: 0BSD OR MIT-0

pcg_t
pcg_seed(pcg_t rng) {
	/* must ensure rng.inc is odd */
	rng.inc = (rng.inc > 0) ? (rng.inc << 1) | 1 : PCG_INCREMENT;
	rng.state += rng.inc;
	pcg_random(&rng);
	return (rng);
}

pcg_t
pcg_getentropy(void) {
	pcg_t rng;
        int getentropy_return = getentropy(&rng, sizeof(rng));
	assert(getentropy_return == 0);
	return (pcg_seed(rng));
}

void
pcg_bytes(pcg_t *restrict rng, void *restrict vptr, size_t size) {
	uint8_t *ptr = vptr;
	while (size > sizeof(pcg_uint_t)) {
		pcg_uint_t rand = pcg_random(rng);
		memcpy(ptr, &rand, sizeof(pcg_uint_t));
		ptr += sizeof(pcg_uint_t);
		size -= sizeof(pcg_uint_t);
	}
	if (size > 0) {
		pcg_uint_t rand = pcg_random(rng);
		memcpy(ptr, &rand, size);
	}
}

/*
 * extern function equivalent to pcg_rand_fast() macro
 */
pcg_uint_t
pcg_rand(pcg_t *rng, pcg_uint_t limit) {
	return (pcg_rand_fast(rng, limit));
}

/*
 * Slow path called from pcg_rand_inline().
 *
 * The `range` of possible values returned by pcg_random() is the same as
 * the number of possible `sample` values. We will return a result if the
 * `sample` is one of `yield` possible values, where `yield` is the largest
 * multiple of `limit` less than `range`. We ensure our results will be
 * unbiased by mapping `yield / limit` possible sample values to each of
 * the `limit` possible return values. If the `sample` is one of the
 * remainder, we `reject` it and resample. The largest multiple makes
 * resampling as rare as possible.
 *
 *	range = 1 << PCG_UINT_BITS
 *	reject = range % limit
 *	yield = range - reject
 */
pcg_uint_t
pcg_rand_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t sample) {
	/*
	 * The value of `range` doesn't fit into a `pcg_uint_t`, so we use
	 * a trick to calculate `reject` without overflow:
	 *
	 * reject =                range % limit
	 *       ==      (range - limit) % limit // equiv modulo limit
	 *       == (pcg_uint_t)(-limit) % limit // equiv modulo range
	 *
	 * The % is safe: we know that `limit` is strictly greater than
	 * zero because of the slow-path guard in pcg_rand_inline().
	 */
	pcg_uint_t reject = -limit % limit;
	/*
	 * Consider separately the set H of possible values of the high word
	 * of the sample, and the set L of possible values of the low word of
	 * the sample. H = { h | 0 <= h < limit }, the set of values we can
	 * return; L is more complicated.
	 *
	 * Because the sample is multiplied by `limit`, the values in L are
	 * spaced apart equally by `limit`. However, because `limit` does not
	 * evenly divide `range`, the set L differs for different H values:
	 * the min and the max in L vary.
	 *
	 * Split the `range` covering L into two spans of size `reject` and
	 * `yield`. The `yield` span always covers exactly `yield / limit`
	 * values spaced apart by `limit`, regardless of their min and max.
	 * This is what we wanted for an unbiased result, so if our L value
	 * lands in the `yield` span outside the `reject` span, we return
	 * our H value; while it's inside the `reject` span we resample.
	 */
	while ((pcg_uint_t)(sample) < reject)
		sample = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
