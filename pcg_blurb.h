// SPDX-License-Identifier: 0BSD OR MIT-0

#pragma once

/*
 * Like pcg_rand(rng, limit) but the fast path is inlined.
 *
 * We use Martin Uecker's arcane tricks to identify at compile time if
 * we can completely omit the slow path because the result of the fast
 * path is trivially unbiased, which happens when the limit is a
 * constant power of two or zero.
 *
 * The expression `limit & limit-1` is zero when the limit is a power
 * of two or zero; when it is cast to `void *` it is either a null
 * pointer constant (always fast) or not (maybe slow).
 *
 * The type of a `?:` expression with a non-void pointer typed branch
 * and a `void *` branch is usually `void *`, unless the `void *` is a
 * null pointer constant, in which case the type of `?:` is the
 * non-void pointer type.
 *
 * A `_Generic()` expression turns the type of the `?:` into a boolean
 * value indicating whether the limit is always fast or maybe slow.
 */
#define pcg_rand_fast(rng, limit)				   \
	pcg_rand_inline(rng, limit,				    \
		_Generic(0 ? (void *)(long)((limit) & ((limit) - 1)) \
			   : (long *) 0, long *: 0, void *: 1))

/**/
