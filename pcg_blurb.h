// SPDX-License-Identifier: 0BSD OR MIT-0

#pragma once

/*
 * Like pcg_rand(rng, limit) but the fast path is inlined.
 *
 * When the limit is a constant power of two the result of the fast path
 * is trivially unbiased, so we should completely omit the slow path.
 * But the compiler can't make this optimization because the slow path
 * guard can be true when the `sample` is a large enough power of two.
 * We use Martin Uecker's arcane C tricks to identify the always-fast
 * case at compile time, which requires a macro to test whether `limit`
 * is the right kind of constant expression.
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
