#pragma once

typedef struct pcg32 {
	uint64_t state, inc;
} pcg32_t;

extern pcg32_t pcg32_randomseed(void);

/* don't call this, call pcg32_uniform() */
extern uint32_t pcg32_uniform_slow(pcg32_t *rng, uint32_t limit, uint64_t num);

/*
 * Melissa O'Neill's pcg32 xsh rr random number generator.
 * (xsh = xor shift, rr = rotate right)
 * This is the "basic" PCG flavour.
 */
static inline uint32_t
pcg32(pcg32_t *rng) {
	/* linear congruential generator */
	uint64_t state = rng->state;
	rng->state = state * 6364136223846793005ULL + rng->inc;
	/* XSH RR (xor shift rotate right) permuted output */
	uint32_t xor = (uint32_t)(((state >> 18) ^ state) >> 27);
	uint32_t rot = (uint32_t)(state >> 59);
	return((xor >> (+rot & 31)) | (xor << (-rot & 31)));
}

/*
 * Daniel Lemire's nearly-divisionless unbiased bounded random numbers.
 *
 * We do a 64-bit multiply `pcg32(rng) * limit` to get a 32.32
 * fixed-point value less than the limit. The result will be the
 * integer part (upper 32 bits), and we will use the fraction part
 * (lower 32 bits) to determine whether or not we need to resample.
 *
 * In the fast path, we avoid doing a division in most cases by
 * comparing the fractional part of `num` with the limit, which is a
 * slight over-estimate for the resample threshold. The slow path does
 * a division to calculate the precise threshold before entering the
 * resample loop.
 */
static inline uint32_t
pcg32_uniform(pcg32_t *rng, uint32_t limit) {
        uint64_t num = (uint64_t)pcg32(rng) * (uint64_t)limit;
        if((uint32_t)(num) < limit)
		return(pcg32_uniform_slow(rng, limit, num));
        return((uint32_t)(num >> 32));
}
