// SPDX-License-Identifier: 0BSD OR MIT-0

#include <stddef.h>
#include <stdint.h>

#include "pcg32.h"
#include "pcg64.h"

/* pcg32_rand() gets slow when the limit approaches 1 << 32 */
#define LARGE (1 << 20)

static inline void
memswap(void *aa, void *bb, size_t size) {
	for (uint8_t *a = aa, *b = bb; size-- > 0; a++, b++) {
		uint8_t c = *a; *a = *b; *b = c;
	}
}

void
pcg32_shuffle(pcg32_t *restrict rng32,
	      void *restrict ptr, size_t count, size_t size) {
	pcg64_t pcg64, *rng64 = &pcg64;
	uint8_t *base = ptr;
	if (count > LARGE) {
		pcg32_bytes(rng32, &pcg64, sizeof(pcg64));
		pcg64 = pcg64_seed(pcg64);
	}
	while (count > LARGE) {
		void *mid = base + size * pcg64_rand_fast(rng64, count);
		void *top = base + size * --count;
		memswap(mid, top, size);
	}
	while (count > 1) {
		void *mid = base + size * pcg32_rand_fast(rng32, count);
		void *top = base + size * --count;
		memswap(mid, top, size);
	}
}
