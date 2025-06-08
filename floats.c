// SPDX-License-Identifier: 0BSD OR MIT-0

/*
 * There are a couple of ways to convert random numbers to a floating
 * point number between 0.0 and 1.0:
 *
 * - Use bit fiddling to construct an integer whose format matches a
 *   float between 1.0 and 2.0; this is the same range as our result
 *   but the exponent is the same over the whole range; bitcast the
 *   value and subtract 1.0.
 *
 * - Shift the integer down to the same range as the mantissa, convert
 *   to float, then multiply by a scaling factor that reduces it to
 *   the desired range.
 *
 * Which is faster?
 *
 * On my Apple M1 Pro and on my AMD Ryzen 7950X, the two kinds of
 * conversion take the same time, and single and double precision
 * are also the same speed.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "pcg32.h"
#include "pcg64.h"

extern float float23(uint32_t u);
extern float float24(uint32_t u);
extern double double52(uint64_t u);
extern double double53(uint64_t u);
extern float pcg32_float23(pcg32_t *rng32);
extern float pcg32_float24(pcg32_t *rng32);
extern double pcg64_double52(pcg64_t *rng64);
extern double pcg64_double53(pcg64_t *rng64);

#ifdef SEPARATE

#define bitcast(t,v) (((union {				\
		static_assert(sizeof(t) == sizeof(v),	\
			      "sizes must match");	\
		__typeof__(v) _v; t _t;			\
	}){ ._v = (v) })._t)

float
float23(uint32_t u) {
	u = ((uint32_t)(127) << 23) | (u >> 9);
        return(bitcast(float, u) - 1.0f);
}

float
float24(uint32_t u) {
	return ((float)(u >> 8) * 0x1.0p-24f);
}

double
double52(uint64_t u) {
	u = ((uint64_t)(1023) << 52) | (u >> 12);
        return(bitcast(double, u) - 1.0);
}

double
double53(uint64_t u) {
	return ((double)(u >> 11) * 0x1.0p-53);
}

float
pcg32_float23(pcg32_t *rng32) {
	uint32_t u = pcg32_random_fast(rng32);
	u = ((uint32_t)(127) << 23) | (u >> 9);
        return(bitcast(float, u) - 1.0f);
}

float
pcg32_float24(pcg32_t *rng32) {
	uint32_t u = pcg32_random_fast(rng32);
	return ((float)(u >> 8) * 0x1.0p-24f);
}

double
pcg64_double52(pcg64_t *rng64) {
	uint64_t u = pcg64_random_fast(rng64);
	u = ((uint64_t)(1023) << 52) | (u >> 12);
        return(bitcast(double, u) - 1.0);
}

double
pcg64_double53(pcg64_t *rng64) {
	uint64_t u = pcg64_random_fast(rng64);
	return ((double)(u >> 11) * 0x1.0p-53);
}



#else

#include "nanotime.h"

static double
check(uint64_t count) {
	uint64_t sum = 0;
	for(uint64_t u = 0; u < count; u++) {
		sum += u;
	}
	return((double)(sum) / (double)(count));
}

static double
speed(uint64_t count, uint64_t t0, uint64_t t1) {
	uint64_t ns = t1 - t0;
	return((double)(ns) / (double)(count));
}

static void time_seq23(void) {
	uint32_t shift = 23;
	uint32_t count = 1 << shift;

	uint64_t t0 = nanotime();

	float sum = 0.0f;
	for(uint32_t u = 0; u < count; u++) {
		sum += float23(u << (32 - shift));
	}

	uint64_t t1 = nanotime();

	printf("23 total %f\n", (double)(sum));
	printf("23 check %f\n", check(count));
	printf("22 speed %f\n", speed(count, t0, t1));
}

static void time_seq24(void) {
	uint32_t shift = 24;
	uint32_t count = 1 << shift;

	uint64_t t0 = nanotime();

	float sum = 0.0f;
	for(uint32_t u = 0; u < count; u++) {
		sum += float24(u << (32 - shift));
	}

	uint64_t t1 = nanotime();

	printf("24 total %f\n", (double)(sum));
	printf("24 check %f\n", check(count));
	printf("24 speed %f\n", speed(count, t0, t1));
}

static void time_seq52(void) {
	// somewhere around a second of run time
	uint64_t shift = 26;
	uint64_t count = 1 << shift;

	uint64_t t0 = nanotime();

	double sum = 0.0;
	for(uint64_t u = 0; u < count; u++) {
		sum += double52(u << (64 - shift));
	}

	uint64_t t1 = nanotime();

	printf("52 total %f\n", (double)(sum));
	printf("52 check %f\n", check(count));
	printf("52 speed %f\n", speed(count, t0, t1));
}

static void time_seq53(void) {
	uint64_t shift = 26;
	uint64_t count = 1 << shift;

	uint64_t t0 = nanotime();

	double sum = 0.0;
	for(uint64_t u = 0; u < count; u++) {
		sum += double53(u << (64 - shift));
	}

	uint64_t t1 = nanotime();

	printf("53 total %f\n", (double)(sum));
	printf("53 check %f\n", check(count));
	printf("52 speed %f\n", speed(count, t0, t1));
}

static void time_rand23(void) {
	pcg32_t rng32[] = { pcg32_getentropy() };
	uint32_t count = 1 << 24;

	uint64_t t0 = nanotime();

	float sum = 0.0f;
	for(uint32_t u = 0; u < count; u++) {
		sum += pcg32_float23(rng32);
	}

	uint64_t t1 = nanotime();

	printf("23 total %f\n", (double)(sum));
	printf("22 speed %f\n", speed(count, t0, t1));
}

static void time_rand24(void) {
	pcg32_t rng32[] = { pcg32_getentropy() };
	uint32_t count = 1 << 24;

	uint64_t t0 = nanotime();

	float sum = 0.0f;
	for(uint32_t u = 0; u < count; u++) {
		sum += pcg32_float24(rng32);
	}

	uint64_t t1 = nanotime();

	printf("24 total %f\n", (double)(sum));
	printf("24 speed %f\n", speed(count, t0, t1));
}

static void time_rand52(void) {
	pcg64_t rng64[] = { pcg64_getentropy() };
	uint32_t count = 1 << 24;

	uint64_t t0 = nanotime();

	double sum = 0.0;
	for(uint64_t u = 0; u < count; u++) {
		sum += pcg64_double52(rng64);
	}

	uint64_t t1 = nanotime();

	printf("52 total %f\n", (double)(sum));
	printf("52 speed %f\n", speed(count, t0, t1));
}

static void time_rand53(void) {
	pcg64_t rng64[] = { pcg64_getentropy() };
	uint32_t count = 1 << 24;

	uint64_t t0 = nanotime();

	double sum = 0.0;
	for(uint64_t u = 0; u < count; u++) {
		sum += pcg64_double53(rng64);
	}

	uint64_t t1 = nanotime();

	printf("53 total %f\n", (double)(sum));
	printf("52 speed %f\n", speed(count, t0, t1));
}

int main(void) {
	pcg32_t rng32[] = { pcg32_getentropy() };
	uint32_t rand = 0;
	for(uint32_t i = 0; i < 0x10000000; i++) {
		rand ^= pcg32_random(rng32);
	}
	printf("warmup %x\n", rand);

	time_seq52();
	time_seq53();
	time_seq24();
	time_seq23();

	time_rand52();
	time_rand53();
	time_rand24();
	time_rand23();
}

#endif
