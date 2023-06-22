// SPDX-License-Identifier: 0BSD OR MIT-0

pcg_t
pcg_seed(pcg_t rng) {
	/* must ensure rng.inc is odd */
	rng.inc = (rng.inc << 1) | 1;
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
pcg_rand_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo) {
	pcg_uint_t residue = -limit % limit;
	while ((pcg_uint_t)(hi_lo) < residue)
		hi_lo = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	return ((pcg_uint_t)(hi_lo >> PCG_UINT_BITS));
}
