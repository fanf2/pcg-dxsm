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

size_t
pcg_totext(char *restrict buf, size_t size, const pcg_t *restrict rng) {
	static const char hex[] = "0123456789abcdef";
	const int shift = sizeof(pcg_ulong_t) * 8 - 4;
	const size_t len = sizeof(pcg_ulong_t) * 4 + 3;
	if (size <= len) return (len);
	pcg_ulong_t v[] = {rng->state, rng->inc};
	for (size_t i = 0; i < 2; i++) {
		for (size_t j = 0; j < 2; j++) {
			for (size_t k = 0; k < sizeof(pcg_ulong_t); k++) {
				*buf++ = hex[v[i] >> shift];
				v[i] <<= 4;
			}
			if (i == 0 || j == 0)
				*buf++ = '-';
		}
	}
	*buf++ = '\0';
	return (len);
}

size_t
pcg_fromtext(const char *restrict cbuf, pcg_t *restrict rng) {
	const unsigned char *ubuf = (const unsigned char *)(cbuf);
	static const signed char unhex[] = {
/*       0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
/* 0 */ -1, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, -2, -2,
/* 1 */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 2 */ -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 3 */ +0, +1, +2, +3, +4, +5, +6, +7, +8, +9, -2, -2, -2, -2, -2, -2,
/* 4 */ -2, 10, 11, 12, 13, 14, 15, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 5 */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 6 */ -2, 10, 11, 12, 13, 14, 15, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 7 */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 8 */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* 9 */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* a */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* b */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* c */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* d */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* e */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
/* f */ -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
	};
	pcg_ulong_t v[] = {0, 0};
	for (size_t i = 0; i < 2; i++) {
		for (size_t j = 0; j < 2; j++) {
			for (size_t k = 0; k < sizeof(pcg_ulong_t); k++) {
				signed char h = unhex[*ubuf++];
				if (h < 0) return (0);
				v[i] = (v[i] << 4) | (pcg_ulong_t)(h);
			}
			if (i == 0 || j == 0)
				if (*ubuf++ != '-') return (0);
		}
	}
	if (unhex[*ubuf++] != -1) return(0);
	*rng = (pcg_t){v[0], v[1]};
	return (sizeof(pcg_ulong_t) * 4 + 3);
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
		sample = (pcg_ulong_t) pcg_random_fast(rng) * limit;
	return ((pcg_uint_t)(sample >> PCG_UINT_BITS));
}
