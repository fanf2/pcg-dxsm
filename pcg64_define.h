// SPDX-License-Identifier: 0BSD OR MIT-0

#define PCG_UINT_BITS	 64
#define pcg_t		 pcg64_t
#define pcg_uint_t	 uint64_t
#define pcg_ulong_t	 __uint128_t
#define pcg_random	 pcg64
#define pcg_seed	 pcg64_seed
#define pcg_entropy	 pcg64_entropy
#define pcg_uniform	 pcg64_uniform
#define pcg_uniform_slow pcg64_uniform_slow
