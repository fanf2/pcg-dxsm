// SPDX-License-Identifier: 0BSD OR MIT-0

#define PCG_UINT_BITS	32
#define PCG_INCREMENT	1442695040888963407ULL

#define pcg_t		pcg32_t
#define pcg_fp_t	float
#define pcg_uint_t	uint32_t
#define pcg_ulong_t	uint64_t

#define pcg_random	pcg32_random
#define pcg_random_fp	pcg32_random_float
#define pcg_rand	pcg32_rand
#define pcg_rand_slow	pcg32_rand_slow
#define pcg_seed	pcg32_seed
#define pcg_getentropy	pcg32_getentropy
