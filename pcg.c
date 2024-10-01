// SPDX-License-Identifier: 0BSD OR MIT-0

pcg_t
pcg_seed(pcg_t rng) {
	const pcg_ulong_t inc = PCG_INCREMENT;
	/* must ensure rng.inc is odd */
	rng.inc = (rng.inc > 0) ? (rng.inc << 1) | 1 : inc;
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
 * The number of possible `sample` values is the same as the `range` of
 * possible values returned by pcg_random(). We will return a result if
 * the `sample` is one of `yield` possible values, where `yield` is the
 * largest multiple of `limit` less than `range`. (The largest multiple
 * makes resampling as rare as possible.) We ensure our results will be
 * unbiased by mapping `yield / limit` of the possible sample values to
 * each of the `limit` possible return values. When the `sample` is one
 * of the remainder, we `reject` it and resample.
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
	 * return. All we need to know about L is its values are spaced apart
	 * equally by `limit` because the sample is multiplied by `limit`
	 * (though the min and max in L vary depending on the value of H).
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
