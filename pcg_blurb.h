// SPDX-License-Identifier: 0BSD OR MIT-0

#pragma once

#define pcg_rand_fast(rng, limit)					 \
	pcg_rand_inline(rng, limit,					  \
		_Generic(0 ? (long *)(0) : (void *)(0 * (long)(limit)),	   \
			long *: /* not 1 << N */ ((limit) & ((limit) - 1)), \
			void *: /* not const */ 1))

