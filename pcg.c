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
	if (getentropy(&rng, sizeof(rng)) < 0)
		err(1, "getentropy");
	return (pcg_seed(rng));
}

pcg_uint_t
pcg_rand(pcg_t *rng, pcg_uint_t limit) {
	return (pcg_rand_fast(rng, limit));
}

pcg_uint_t
pcg_rand_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo) {
	pcg_uint_t residue = -limit % limit;
	while ((pcg_uint_t)(hi_lo) < residue)
		hi_lo = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	return ((pcg_uint_t)(hi_lo >> PCG_UINT_BITS));
}

void
pcg_random_bytes(pcg_t *restrict rng, void *restrict vptr, size_t size) {
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
