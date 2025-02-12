// SPDX-License-Identifier: 0BSD OR MIT-0

#include <stdio.h>
#include <stdint.h>

static inline uint64_t
lcg64(uint64_t state, uint64_t inc) {
	return (6364136223846793005ULL * state + inc);
}

static void
print_lcg64_defines(int max, char *name, uint64_t state, uint64_t inc) {
	for (int i = 0; i <= max; i++) {
		printf("#define %s%d %zuuLL\n", name, i, (size_t)state);
		state = lcg64(state, inc);
	}
}

int main(void) {
	print_lcg64_defines(8, "MULs", 1, 0);
	print_lcg64_defines(8, "MULi", 0, 1);
}

/*
 * To speed up a loop that generates a series of values from PCG, we
 * want to avoid data dependencies between successive iterations so
 * that they can be calculated at the same time using instruction-
 * level parallelism, or SIMD vector instructions.
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
