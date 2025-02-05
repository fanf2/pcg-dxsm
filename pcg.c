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

void
pcg_shuffle(
	pcg_t *restrict rng, void *restrict ptr, pcg_uint_t count, size_t size
) {
	uint8_t *base = ptr;
	while (count > 1) {
		uint8_t *mid = base + size * pcg_rand_fast(rng, count);
		uint8_t *top = base + size * --count;
		for (size_t i = 0; i < size; i++) {
			uint8_t swap = mid[i];
			mid[i] = top[i];
			top[i] = swap;
		}
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
 * Slow path called from pcg_rand_inline(). Re-check and return `sample`
 * if it passes, or re-try with a new sample.
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
 * it is one of the `reject` values that cause a re-try.
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
	 * The upper half of `sample` has `limit` possible values; for each
	 * upper-half value U, the lower half has a value of the form
	 *
	 *	L = a + b * limit
	 *
	 * Lower-half values are spaced `limit` apart by the multiplication.
	 * Depending on U, `b` (and therefore L) has `quota` or `quota + 1`
	 * possible values. The alignment `a` is determined by U.
	 *
	 * We split the `range` covering L into two spans of size `yield`
	 * and `reject`. The `yield` span is a multiple of `limit` so it
	 * always covers exactly `quota` possible values of L, regardless
	 * of the alignment; we return these unbiased samples. For some
	 * values of U, one over-quota value of L can also fall in the
	 * `reject` span; when we get one of these samples we re-try.
	 */
	while ((pcg_uint_t)(sample) < reject)
		sample = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
