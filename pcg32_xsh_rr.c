// SPDX-License-Identifier: 0BSD OR MIT-0

static inline uint32_t
pcg32_random(pcg32_t *rng) {
	/* linear congruential generator */
	uint64_t state = rng->state;
	rng->state = state * 6364136223846793005ULL + rng->inc;
	/* XSH RR (xor shift random rotate) permuted output */
	uint32_t xsh = (uint32_t)(((state >> 18) ^ state) >> 27);
	uint32_t rot = (uint32_t)(state >> 59);
	return ((xsh >> (+rot & 31)) | (xsh << (-rot & 31)));
}

static inline float
pcg32_float(pcg32_t *rng) {
	return ((float)(pcg32_random(rng) >> 8) * 0x1.0p-24f);
}

extern void pcg32_shuffle(
	pcg32_t *restrict rng, void *restrict ptr, size_t count, size_t size);
