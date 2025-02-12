// SPDX-License-Identifier: 0BSD OR MIT-0

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

typedef unsigned char byte;

#define VEC_CAST(type, val)			\
	__builtin_convertvector(val, type)

#define VEC_TYPE(type, size)					\
	type __attribute__((vector_size(sizeof(type) * size)))

#include "pcg32.h"
#include "pcg32-mul.h"
#include "pcg32-vec.h"

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

struct {
	char *name;
	pcg32_bytes_fn *bytes;
	double pop, mean, sigma;
} measure[] = {
	{ "u1", pcg32_bytes_u1, 0.0, 0.0, 0.0 },
	{ "u2", pcg32_bytes_u2, 0.0, 0.0, 0.0 },
	{ "u3", pcg32_bytes_u3, 0.0, 0.0, 0.0 },
	{ "u4", pcg32_bytes_u4, 0.0, 0.0, 0.0 },
	{ "x2", pcg32_bytes_x2, 0.0, 0.0, 0.0 },
	{ "x4", pcg32_bytes_x4, 0.0, 0.0, 0.0 },
	{ "x8", pcg32_bytes_x8, 0.0, 0.0, 0.0 },
};

#define SIZE 4096
#define COUNT 16

int main(void) {
	byte check[SIZE];
	byte buf[SIZE];

	pcg32_t rng = pcg32_seed((pcg32_t){0});
	pcg32_bytes(&rng, check, SIZE);
	pcg32_bytes(&rng, buf, SIZE);

	size_t fns = sizeof(measure) / sizeof(measure[0]);

	for (;;) {
		size_t fn = pcg32_rand(&rng, fns);
		if(measure[fn].pop == COUNT)
			continue;

		pcg32_t rng0 = pcg32_seed((pcg32_t){0});

		__sync_synchronize();
		uint64_t t0 = nanotime();

		measure[fn].bytes(&rng0, buf, SIZE);

		__sync_synchronize();
		uint64_t t1 = nanotime();

		assert(memcmp(buf, check, SIZE) == 0);

		uint64_t ns = t1 - t0;
		double speed = (double)SIZE / (double)ns;

		double pop = measure[fn].pop;
		double mean = measure[fn].mean;
		double sigma = measure[fn].sigma;

		double delta = speed - mean;

		pop += 1;
		mean += delta / pop;
		sigma += delta * (speed - mean);

		measure[fn].pop = pop;
		measure[fn].mean = mean;
		measure[fn].sigma = sigma;

		putchar('.');
		fflush(stdout);

		bool done = true;
		for (size_t fn = 0; fn < fns; fn++) {
			if (measure[fn].pop < COUNT) {
				done = false;
			}
		}
		if (done) break;
	}
	putchar('\n');

	for (size_t fn = 0; fn < fns; fn++) {
		printf("%s %5.2f +/- %.2f bytes/ns x %.2f\n",
		       measure[fn].name,
		       measure[fn].mean,
		       sqrt(measure[fn].sigma),
		       measure[fn].mean / measure[0].mean);
	}
}
