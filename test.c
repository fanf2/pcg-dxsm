// SPDX-License-Identifier: 0BSD OR MIT-0

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcg32.h"
#include "pcg64.h"

/* exercise the pcg32_shuffle() large array case */
#define LARGER (1 << 24)

static void
test_shuffle(pcg32_t *rng) {
	uint32_t *a;
	a = malloc(LARGER * sizeof(*a));
	assert(a != NULL);
	uint64_t sum = 0;
	for (uint32_t i = 0; i < LARGER; i++) {
		a[i] = i;
		sum += i;
	}
	pcg32_shuffle(rng, a, LARGER, sizeof(*a));
	bool sorted = true;
	for (uint32_t i = 0; i < LARGER; i++) {
		sum -= a[i];
		if (a[i] != i)
			sorted = false;
	}
	assert(sum == 0);
	assert(sorted == false);
	free(a);
}

int
main(void) {
	pcg32_t rng32 = pcg32_getentropy();
	assert(pcg32_rand(&rng32, 0) == 0);
	printf("mask %" PRIX32 "\n", pcg32_rand_fast(&rng32, 1UL << 20));
	printf("fast %" PRIX32 "\n", pcg32_rand_fast(&rng32, INT32_MAX));
	printf("slow %" PRIX32 "\n", pcg32_rand(&rng32, INT32_MAX));
	printf("fp %.8f\n", pcg32_float(&rng32));

	pcg64_t rng64 = pcg64_getentropy();
	assert(pcg64_rand(&rng64, 0) == 0);
	printf("mask %" PRIX64 "\n", pcg64_rand_fast(&rng64, 1ULL << 40));
	printf("fast %" PRIX64 "\n", pcg64_rand_fast(&rng64, INT64_MAX));
	printf("slow %" PRIX64 "\n", pcg64_rand(&rng64, INT64_MAX));
	printf("fp %.16f\n", pcg64_double(&rng64));

	test_shuffle(&rng32);
}
