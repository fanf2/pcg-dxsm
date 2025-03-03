// SPDX-License-Identifier: 0BSD OR MIT-0

pcg_t
pcg_seed(pcg_t rng) {
	const pcg_ulong_t inc = PCG_INCREMENT;
	/* must ensure rng.inc is odd */
	rng.inc = (rng.inc > 0) ? (rng.inc << 1) | 1 : inc;
	rng.state += rng.inc;
	pcg_random_fast(&rng);
	return (rng);
}

pcg_t
pcg_getentropy(void) {
	pcg_t rng;
	int getentropy_return = getentropy(&rng, sizeof(rng));
	/* failure is not an option */
	assert(getentropy_return == 0);
	return (pcg_seed(rng));
}

void
pcg_bytes(pcg_t *restrict rng, void *restrict ptr, size_t size) {
	uint8_t *dest = ptr;
	while (size >= sizeof(pcg_uint_t)) {
		pcg_uint_t rand = pcg_random_fast(rng);
		memcpy(dest, &rand, sizeof(pcg_uint_t));
		dest += sizeof(pcg_uint_t);
		size -= sizeof(pcg_uint_t);
	}
	if (size > 0) {
		pcg_uint_t rand = pcg_random_fast(rng);
		memcpy(dest, &rand, size);
	}
}

void
pcg_shuffle(
	pcg_t *restrict rng, void *restrict ptr, pcg_uint_t count, size_t size
) {
	uint8_t swap_x, *base = ptr;
	while (count > 1) {
		uint8_t *mid = base + size * pcg_rand_fast(rng, count);
		uint8_t *top = base + size * --count;
		for (size_t x = 0; x < size; x++) {
			swap_x = mid[x]; mid[x] = top[x]; top[x] = swap_x;
		}
	}
}

pcg_uint_t
pcg_random_small(pcg_t *rng) {
	return (pcg_random_fast(rng));
}

pcg_uint_t
pcg_rand_small(pcg_t *rng, pcg_uint_t limit) {
	return (pcg_rand_fast(rng, limit));
}

pcg_uint_t
pcg_rand_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t sample) {
	/*
	 * This % is safe: we know that `limit` is strictly greater than
	 * zero because of the slow-path guard in pcg_rand_fast().
	 */
	pcg_uint_t reject = -limit % limit;
	while ((pcg_uint_t)(sample) < reject)
		sample = pcg_ulong_biased(rng, limit);
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
