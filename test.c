// SPDX-License-Identifier: 0BSD OR MIT-0

#include <inttypes.h>
#include <stdio.h>

#include "pcg32.h"
#include "pcg_undef.h"
#include "pcg64.h"
#include "pcg_undef.h"

int
main(void) {
	pcg32_t rng32 = pcg32_entropy();
	pcg64_t rng64 = pcg64_entropy();
	printf("%" PRIu32 "\n", pcg32_uniform(&rng32, INT32_MAX));
	printf("%" PRIu64 "\n", pcg64_uniform(&rng64, INT64_MAX));
	printf("%f\n", pcg32_float(&rng32));
	printf("%f\n", pcg64_double(&rng64));
}
