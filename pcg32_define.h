/*
 * Written by Tony Finch <dot@dotat.at> in Cambridge.
 *
 * Permission is hereby granted to use, copy, modify, and/or
 * distribute this software for any purpose with or without fee.
 *
 * This software is provided 'as is', without warranty of any kind.
 * In no event shall the authors be liable for any damages arising
 * from the use of this software.
 *
 * SPDX-License-Identifier: 0BSD OR MIT-0
 */

#define PCG_UINT_BITS	 32
#define pcg_t		 pcg32_t
#define pcg_uint_t	 uint32_t
#define pcg_ulong_t	 uint64_t
#define pcg_random	 pcg32
#define pcg_seed	 pcg32_seed
#define pcg_entropy	 pcg32_entropy
#define pcg_uniform	 pcg32_uniform
#define pcg_uniform_slow pcg32_uniform_slow
