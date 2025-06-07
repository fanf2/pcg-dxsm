// SPDX-License-Identifier: 0BSD OR MIT-0

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pcg32.h"
#include "pcg64.h"

static void
test_shuffle64(pcg64_t *rng) {
	/* pcg64 has 256 bits of state; 2^256 == 10^77 > 10^68 == 52! */
	size_t suits = 4;
	size_t ranks = 13;
	size_t count = suits * ranks;
	char deck[] =
		" A♣︎ 2♣︎ 3♣︎ 4♣︎ 5♣︎ 6♣︎ 7♣︎ 8♣︎ 9♣︎ 0♣︎ J♣︎ Q♣︎ K♣︎"
		" A♦︎ 2♦︎ 3♦︎ 4♦︎ 5♦︎ 6♦︎ 7♦︎ 8♦︎ 9♦︎ 0♦︎ J♦︎ Q♦︎ K♦︎"
		" A♥︎ 2♥︎ 3♥︎ 4♥︎ 5♥︎ 6♥︎ 7♥︎ 8♥︎ 9♥︎ 0♥︎ J♥︎ Q♥︎ K♥︎"
		" A♠︎ 2♠︎ 3♠︎ 4♠︎ 5♠︎ 6♠︎ 7♠︎ 8♠︎ 9♠︎ 0♠︎ J♠︎ Q♠︎ K♠︎";
	size_t len = sizeof(deck) - 1;
	size_t size = len / count;
	pcg64_shuffle(rng, deck, count, size);
	for (size_t suit = 0; suit < suits; suit++) {
		char *row = deck + suit * ranks * size;
		for (size_t rank = 0; rank < ranks; rank++) {
			printf("%.*s", (int)size, row + rank * size);
		}
		printf("\n");
	}
}

static void
test_shuffle32(pcg32_t *rng) {
	uint32_t shuffle = 1 << 24;
	uint32_t *a;
	a = malloc(shuffle * sizeof(*a));
	assert(a != NULL);
	uint64_t sum = 0;
	for (uint32_t i = 0; i < shuffle; i++) {
		a[i] = i;
		sum += i;
	}
	pcg32_shuffle(rng, a, shuffle, sizeof(*a));
	bool sorted = true;
	for (uint32_t i = 0; i < shuffle; i++) {
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
	pcg32_t rng32[] = { pcg32_getentropy() };
	uint32_t var32 = pcg32_random(rng32);
	printf("zero  %" PRIX32 "\n", pcg32_rand(rng32, 0));
	printf("mask  %" PRIX32 "\n", pcg32_rand(rng32, 1UL << 20));
	printf("prime %" PRIX32 "\n", pcg32_rand(rng32, (1UL << 19) - 1));
	printf("var   %" PRIX32 "\n", pcg32_rand(rng32, var32));
	printf("fast  %" PRIX32 "\n", pcg32_rand_fast(rng32, INT32_MAX));
	printf("small %" PRIX32 "\n", pcg32_rand_small(rng32, INT32_MAX));
	printf("fp %.8f\n", (double)pcg32_float(rng32));

	pcg64_t rng64[] = { pcg64_getentropy() };
	uint64_t var64 = pcg64_random(rng64);
	printf("zero  %" PRIX64 "\n", pcg64_rand(rng64, 0));
	printf("mask  %" PRIX64 "\n", pcg64_rand(rng64, 1ULL << 40));
	printf("prime %" PRIX64 "\n", pcg64_rand(rng64, (1ULL << 31) - 1));
	printf("var   %" PRIX64 "\n", pcg64_rand(rng64, var64));
	printf("fast  %" PRIX64 "\n", pcg64_rand_fast(rng64, INT64_MAX));
	printf("small %" PRIX64 "\n", pcg64_rand_small(rng64, INT64_MAX));
	printf("fp %.16f\n", pcg64_double(rng64));

	test_shuffle32(rng32);
	test_shuffle64(rng64);
}
