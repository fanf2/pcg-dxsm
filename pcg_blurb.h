// SPDX-License-Identifier: 0BSD OR MIT-0

#pragma once

#if defined(__NO_INLINE__) || defined(__OPTIMIZE_SIZE__)
#define pcg_random pcg_random_small
#define pcg_rand   pcg_rand_small
#else
#define pcg_random pcg_random_fast

/*
 * Select an inline version of pcg_rand() specialized for a run-time
 * variable or a compile-time constant limit, using Martin Uecker's
 * arcane C tricks. The type of a `?:` expression that has a non-void
 * pointer-typed branch and a `void *` branch is usually `void *`. But
 * if the `void *` is a null pointer constant, the type of `?:` is the
 * non-void pointer type. We use `!limit` so that only `limit != 0`
 * becomes a null pointer constant, so that we only pass a nonzero
 * constant limit to a function that immediately calculates % limit.
 */
#define pcg_rand(rng, limit)					\
	_Generic(0L ? (long *) 0L : (void *)(long)!(limit),	\
		long *: pcg_rand_const(rng, limit),		\
		void *: pcg_rand_fast(rng, limit))

#endif

/**/
