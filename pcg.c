/*
 * Written by Tony Finch <dot@dotat.at> in Cambridge.
 *
 * Permission is hereby granted to use, copy, modify, and/or
 * distribute this software for any purpose with or without fee.
 *
 * This software is provided 'as is', without warranty of any kind.
 * In no event shall the authors be liable for any damages arising
 * from the use of this software.
 *
 * SPDX-License-Identifier: 0BSD OR MIT-0
 */

pcg_t
pcg_seed(pcg_t rng) {
	rng.inc = (rng.inc << 1) | 1;
	rng.state += rng.inc;
	pcg_random(&rng);
	return (rng);
}

pcg_t
pcg_entropy(void) {
	pcg_t rng;
	if (getentropy(&rng, sizeof(rng)) < 0)
		err(1, "getentropy");
	return (pcg_seed(rng));
}

pcg_uint_t
pcg_uniform_slow(pcg_t *rng, pcg_uint_t limit, pcg_ulong_t hi_lo) {
	pcg_uint_t residue = -limit % limit;
	while ((pcg_uint_t)(hi_lo) < residue)
		hi_lo = (pcg_ulong_t)pcg_random(rng) * (pcg_ulong_t)limit;
	return ((pcg_uint_t)(hi_lo >> PCG_UINT_BITS));
}
