// SPDX-License-Identifier: 0BSD OR MIT-0

#include <inttypes.h>
#include <stdio.h>

#include "pcg32.h"
#include "pcg_undef.h"
#include "pcg64.h"
#include "pcg_undef.h"

int
main(void) {
	pcg32_t rng32 = pcg32_getentropy();
	pcg64_t rng64 = pcg64_getentropy();
	printf("%" PRIu32 "\n", pcg32_rand(&rng32, INT32_MAX));
	printf("%" PRIu64 "\n", pcg64_rand(&rng64, INT64_MAX));
	printf("%f\n", pcg32_random_float(&rng32));
	printf("%f\n", pcg64_random_double(&rng64));
}
