// SPDX-License-Identifier: 0BSD OR MIT-0

#if !defined(VEC_EXPAND)

#include <stdio.h>

static void
define(char *name) {
	printf("\n#undef  %s", name);
	printf("\n#define %s ", name);
}

static void
vec_mul(char *name, int size) {
	for (int i = 0; i < size; i++) {
		printf(" %s%d,", name, i);
	}
}

static void
expand(int size) {
	define("VEC_SIZE");       printf("%d", size);
	define("pcg32_bytes_xV"); printf("pcg32_bytes_x%d", size);
	define("pcg32_rand_xV");  printf("pcg32_rand_x%d", size);
	define("uint32xV_t");     printf("uint32x%d_t", size);
	define("uint64xV_t");     printf("uint64x%d_t", size);
	define("MULsV");          printf("MULs%d", size);
	define("MULiV");          printf("MULi%d", size);
	define("VEC_MULs");       vec_mul("MULs", size);
	define("VEC_MULi");       vec_mul("MULi", size);
	printf("\n#include \"pcg32-vec.c\"\n");
}

int main(void) {
	printf("#define VEC_EXPAND\n");
	expand(2);
	expand(4);
	expand(8);
}

#else

/*
 * this section is repeatedly macro-expanded into pcg32-vec.h
 */

typedef VEC_TYPE(uint32_t, VEC_SIZE) uint32xV_t;
typedef VEC_TYPE(uint64_t, VEC_SIZE) uint64xV_t;

static inline uint32xV_t
pcg32_rand_xV(pcg32_t *rng) {
	/* skip ahead multipliers */
	const uint64xV_t muls = { VEC_MULs };
	const uint64xV_t muli = { VEC_MULi };
	/* parallel linear congruential generator */
	uint64xV_t state = rng->state * muls + rng->inc * muli;
	rng->state = rng->state * MULsV + rng->inc * MULiV;
	/* XSH RR (xor shift random rotate) permuted output */
	uint64xV_t xsh64 = (((state >> 18) ^ state) >> 27);
	uint64xV_t rot64 = (state >> 59);
	uint32xV_t xsh = VEC_CAST(uint32xV_t, xsh64);
	uint32xV_t rot = VEC_CAST(uint32xV_t, rot64);
	return ((xsh >> (+rot & 31)) | (xsh << (-rot & 31)));
}

static void
pcg32_bytes_xV(pcg32_t *restrict prng, void *restrict ptr, size_t size) {
	pcg32_t rng = *prng;
	byte *dest = ptr;
	while (size >= sizeof(uint32xV_t)) {
		uint32xV_t rand = pcg32_rand_xV(&rng);
		memcpy(dest, &rand, sizeof(uint32xV_t));
		dest += sizeof(uint32xV_t);
		size -= sizeof(uint32xV_t);
	}
	prng->state = rng.state;
}

#endif
