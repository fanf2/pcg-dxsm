// SPDX-License-Identifier: 0BSD OR MIT-0

/*
 * Can we speed up a loop that generates a series of values from PCG by
 * avoiding data dependencies between successive iterations?
 *
 * The idea is to calculate several outputs from the same PCG sequence
 * at the same time, to benefit from greater instruction-level
 * parallelism and/or SIMD vector instructions.
 *
 * How much speedup can we get, and what shape of unrolling works best?
 *
 * The equations below illustrate the main data dependency in PCG. (I'm
 * using value[x] as a mathematical subscript to indicate successive
 * values.) Each value of the LCG depends on its predecessor so they
 * must be calculated sequentially:
 *
 *        state[b] = lcg(state[a], inc)
 *        state[c] = lcg(state[b], inc)
 *
 * We can avoid this dependency by using skip-ahead multipliers, which
 * allow successive states to be calculated in parallel, all starting
 * from the initial state. (The disadvantage is that we need more
 * multiplications.)
 *
 * We define the iterative calculation of the state as follows:
 *
 *        lcg(s, inc) = s * mul + inc
 *
 *        state[k] = lcg(state[j], inc)
 *
 *        k = j + 1
 *
 * The skip-ahead multipliers are defined as
 *
 *        muls[0] = 1,  muls[k] = lcg(muls[j], 0)
 *        muli[0] = 0,  muli[k] = lcg(muli[j], 1)
 *
 * The skip-ahead multipliers depend only on the fixed LCG multiplier,
 * not the variable state nor the configurable increment, so they can be
 * precalculated.
 *
 * The skip-ahead multipliers give us a direct way to calculate a later
 * state from the initial state without iterating through all the
 * intermediate states, like this:
 *
 *        state[k] = state[0] * muls[k] + inc * muli[k]
 *
 * We can see that this is equivalent to the iterative equation for
 * state[k] by induction.
 *
 * Expand each part of the direct calculation using the definitions of
 * the skip-ahead multipliers and the LCG:
 *
 *        state[0] * muls[k]
 *   ==   state[0] * lcg(muls[j], 0)
 *   ==   state[0] * (muls[j] * mul + 0)
 *   ==   (state[0] * muls[j]) * mul
 *
 *        inc * muli[k]
 *   ==   inc * lcg(muli[j], 1)
 *   ==   inc * (muli[j] * mul + 1)
 *   ==   (inc * muli[j]) * mul + inc
 *
 * Bring the parts back together:
 *
 *        state[k]
 *
 *   ==   state[0] * muls[k] + inc * muli[k]
 *
 *   ==   (state[0] * muls[j]) * mul +
 *        (inc * muli[j]) * mul + inc
 *
 *   ==   (state[0] * muls[j] + inc * muli[j]) * mul + inc
 *
 * Substitute in the definition of the j'th direct calculation, and the
 * definition of the LCG, and we get the equivalence we wanted:
 *
 *   ==   state[j] * mul + inc
 *
 *   ==   lcg(state[j], inc)
 *
 * For the base case we have,
 *
 *        state[0] * muls[0] + inc * muli[0]
 *   ==   state[0] * 1 + inc * 0
 *   ==   state[0]
 *
 * as required. QED.
 */

#if defined(GENERATE)

/*
 * This program is compiled twice: the first section here does some
 * code generation before the actual benchmark is compiled from the
 * section below.
 */

#include <err.h>
#include <stdio.h>
#include <stdint.h>

/*
 * generate skip-ahead multipliers
 */

static inline uint64_t
lcg64(uint64_t state, uint64_t inc) {
	return (6364136223846793005ULL * state + inc);
}

static void
lcg64_mul(FILE *fp, int max, char *name, uint64_t state, uint64_t inc) {
	for (int i = 0; i <= max; i++) {
		fprintf(fp, "#define %s%d %lluuLL\n",
			name, i, (unsigned long long)state);
		state = lcg64(state, inc);
	}
	fprintf(fp, "\n");
}

static void
generate_mul_h(void) {
	FILE *fp_mul = fopen("bytes-mul.h", "w");
	if (fp_mul == NULL) {
		err(1, "open bytes-mul.h");
	}

	lcg64_mul(fp_mul, 16, "MULs", 1, 0);
	lcg64_mul(fp_mul, 16, "MULi", 0, 1);

	if (fclose(fp_mul) < 0) {
		err(1, "write bytes-mul.h");
	}
}

/*
 * macro-expand vectorized implementations of pcg32_bytes()
 */

static void
define(FILE *fp, char *name) {
	fprintf(fp, "\n#undef  %s", name);
	fprintf(fp, "\n#define %s ", name);
}

static void
vec_mul(FILE *fp, char *name, int size) {
	for (int i = 0; i < size; i++) {
		fprintf(fp, " %s%d,", name, i);
	}
}

static void
expand(FILE *fp, int size) {
	define(fp, "VEC_SIZE");       fprintf(fp, "%d", size);
	define(fp, "pcg32_bytes_xV"); fprintf(fp, "pcg32_bytes_x%d", size);
	define(fp, "pcg32_rand_xV");  fprintf(fp, "pcg32_rand_x%d", size);
	define(fp, "uint32xV_t");     fprintf(fp, "uint32x%d_t", size);
	define(fp, "uint64xV_t");     fprintf(fp, "uint64x%d_t", size);
	define(fp, "MULsV");          fprintf(fp, "MULs%d", size);
	define(fp, "MULiV");          fprintf(fp, "MULi%d", size);
	define(fp, "VEC_MULs");       vec_mul(fp, "MULs", size);
	define(fp, "VEC_MULi");       vec_mul(fp, "MULi", size);
	fprintf(fp, "\n#include \"bytes.c\"\n");
}

static void
generate_vec_h(void) {
	FILE *fp_vec = fopen("bytes-vec.h", "w");
	if (fp_vec == NULL) {
		err(1, "open bytes-vec.h");
	}

	fprintf(fp_vec, "#define VEC_EXPAND\n");
	expand(fp_vec, 2);
	expand(fp_vec, 4);
	expand(fp_vec, 8);
	expand(fp_vec, 16);

	if (fclose(fp_vec) < 0) {
		err(1, "write bytes-vec.h");
	}
}

/*
 * This first main() drives the compile-time code generation;
 * there's a second main() below which runs the benchmarks.
 */

int main(void) {
	generate_mul_h();
	generate_vec_h();
}

#elif defined(VEC_EXPAND)

/*
 * this section is repeatedly macro-expanded into bytes-vec.h
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

#else

/*
 * the benchmark code
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned char byte;

#define VEC_CAST(type, val)			\
	__builtin_convertvector(val, type)

#define VEC_TYPE(type, size)					\
	type __attribute__((vector_size(sizeof(type) * size)))

#include "pcg32.h"

/*
 * bytes.h is combined from bytes-mul.h and bytes-vec.h by the Makefile
 */
#include "bytes.h"

static inline void
put_uint32(byte *dest, size_t off, uint32_t word) {
	memcpy(dest + off * sizeof(word), &word, sizeof(word));
}

static inline uint32_t
xsh_rr(uint64_t state) {
	uint32_t xsh = (uint32_t)(((state >> 18) ^ state) >> 27);
	uint32_t rot = (uint32_t)(state >> 59);
	return ((xsh >> (+rot & 31)) | (xsh << (-rot & 31)));
}

/*
 * four copies of the same thing slightly bigger each time
 */

static void
pcg32_bytes_u1(pcg32_t *restrict rng, void *restrict ptr, size_t size) {
	uint64_t state = rng->state;
	uint64_t inc = rng->inc;
	byte *dest = ptr;
	while (size >= sizeof(uint32_t) * 1) {
		put_uint32(dest, 0, xsh_rr(state * MULs0 + inc * MULi0));
		state =                    state * MULs1 + inc * MULi1;
		dest += sizeof(uint32_t) * 1;
		size -= sizeof(uint32_t) * 1;
	}
	rng->state = state;
}

static void
pcg32_bytes_u2(pcg32_t *restrict rng, void *restrict ptr, size_t size) {
	uint64_t state = rng->state;
	uint64_t inc = rng->inc;
	byte *dest = ptr;
	while (size >= sizeof(uint32_t) * 2) {
		put_uint32(dest, 0, xsh_rr(state * MULs0 + inc * MULi0));
		put_uint32(dest, 1, xsh_rr(state * MULs1 + inc * MULi1));
		state =                    state * MULs2 + inc * MULi2;
		dest += sizeof(uint32_t) * 2;
		size -= sizeof(uint32_t) * 2;
	}
	rng->state = state;
}

static void
pcg32_bytes_u3(pcg32_t *restrict rng, void *restrict ptr, size_t size) {
	uint64_t state = rng->state;
	uint64_t inc = rng->inc;
	byte *dest = ptr;
	while (size >= sizeof(uint32_t) * 3) {
		put_uint32(dest, 0, xsh_rr(state * MULs0 + inc * MULi0));
		put_uint32(dest, 1, xsh_rr(state * MULs1 + inc * MULi1));
		put_uint32(dest, 2, xsh_rr(state * MULs2 + inc * MULi2));
		state =                    state * MULs3 + inc * MULi3;
		dest += sizeof(uint32_t) * 3;
		size -= sizeof(uint32_t) * 3;
	}
	rng->state = state;
}

static void
pcg32_bytes_u4(pcg32_t *restrict rng, void *restrict ptr, size_t size) {
	uint64_t state = rng->state;
	uint64_t inc = rng->inc;
	byte *dest = ptr;
	while (size >= sizeof(uint32_t) * 4) {
		put_uint32(dest, 0, xsh_rr(state * MULs0 + inc * MULi0));
		put_uint32(dest, 1, xsh_rr(state * MULs1 + inc * MULi1));
		put_uint32(dest, 2, xsh_rr(state * MULs2 + inc * MULi2));
		put_uint32(dest, 3, xsh_rr(state * MULs3 + inc * MULi3));
		state =                    state * MULs4 + inc * MULi4;
		dest += sizeof(uint32_t) * 4;
		size -= sizeof(uint32_t) * 4;
	}
	rng->state = state;
}

#ifdef __APPLE__

/*
 * need a nonstandard API to get nanosecond resolution, sigh
 */

#include <mach/mach_time.h>

static uint64_t
nanotime(void) {
	static mach_timebase_info_data_t scale;
	if (scale.denom == 0) {
		kern_return_t status = mach_timebase_info(&scale);
		assert(status == KERN_SUCCESS);
	}
	return (mach_absolute_time() * scale.numer / scale.denom);
}

#else

#define NS_PER_S (1000*1000*1000)

static uint64_t
nanotime(void) {
	struct timespec tv;
	assert(clock_gettime(CLOCK_MONOTONIC, &tv) == 0);
	return((uint64_t)tv.tv_sec * NS_PER_S + (uint64_t)tv.tv_nsec);
}

#endif

typedef void pcg32_bytes_fn(
	pcg32_t *restrict rng, void *restrict ptr, size_t size);

static struct {
	char *name;
	pcg32_bytes_fn *bytes;
	double speed;
} measure[] = {
	{ "__", pcg32_bytes, 0.0 },
	{ "u1", pcg32_bytes_u1, 0.0 },
	{ "u2", pcg32_bytes_u2, 0.0 },
	{ "u3", pcg32_bytes_u3, 0.0 },
	{ "u4", pcg32_bytes_u4, 0.0 },
	{ "x2", pcg32_bytes_x2, 0.0 },
	{ "x4", pcg32_bytes_x4, 0.0 },
	{ "x8", pcg32_bytes_x8, 0.0 },
	{ "16", pcg32_bytes_x16, 0.0 },
};

/*
 * needs to be a multiple of 3 so that pcg32_bytes_u3() fills it completely
 */
#define SIZE 3072

#define SENSIBLE_TIME (333*1000*1000)

int main(void) {
	size_t fns = sizeof(measure) / sizeof(measure[0]);

	pcg32_t rng0 = pcg32_getentropy();

	byte *check = malloc(SIZE);
	byte *buf = malloc(SIZE);

	assert(check != NULL);
	assert(buf != NULL);

	pcg32_t rng = rng0;

	uint64_t t0 = nanotime();
	pcg32_bytes(&rng, buf, SIZE);
	uint64_t t1 = nanotime();

	uint64_t ns = t1 - t0;
	size_t iters = SENSIBLE_TIME / ns;

	pcg32_t rngN = rng0;

	for (size_t fn = 0; fn < fns; fn++) {
		memset(buf, 0, SIZE);
		rng = rng0;

		__sync_synchronize();
		uint64_t t0 = nanotime();

		for (size_t i = 0; i < iters; i++) {
			measure[fn].bytes(&rng, buf, SIZE);
		}

		__sync_synchronize();
		uint64_t t1 = nanotime();

		if (fn == 0) {
			memmove(check, buf, SIZE);
			rngN = rng;
		} else {
			assert(memcmp(check, buf, SIZE) == 0);
			assert(memcmp(&rngN, &rng, sizeof(rng)) == 0);
		}

		uint64_t ns = t1 - t0;
		measure[fn].speed = (double)(SIZE * iters) / (double)ns;

		printf("%s %5.2f bytes/ns x %.2f\n",
		       measure[fn].name,
		       measure[fn].speed,
		       measure[fn].speed / measure[0].speed);
	}
}

#endif
