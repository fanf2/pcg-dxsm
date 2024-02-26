// SPDX-License-Identifier: 0BSD OR MIT-0

#define PCG_UINT_BITS	64
#define PCG_INCREMENT	((pcg_ulong_t)6364136223846793005ULL << 64 |\
			 (pcg_ulong_t)1442695040888963407ULL)

#define pcg_t		pcg64_t
#define pcg_fp_t	double
#define pcg_uint_t	uint64_t
#define pcg_ulong_t	__uint128_t

#define pcg_random	pcg64_random
#define pcg_random_fp	pcg64_random_double
#define pcg_rand	pcg64_rand
#define pcg_rand_slow	pcg64_rand_slow
#define pcg_seed	pcg64_seed
#define pcg_getentropy	pcg64_getentropy
