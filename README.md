Uniform random numbers with PCG
===============================

This repository contains implementations in C of the preferred 32-bit
and 64-bit variants of [Melissa O'Neill's PCG family of random number
algorithms][pcg], with Daniel Lemire's nearly-divisionless algorithm
for unbiased bounded random numbers.

[pcg]: https://www.pcg-random.org/


permuted congruential generator
-------------------------------

The PCG functions are constructed from a collection of linear
congruential generators and a collection of output permutations.

A linear congruential random number generator looks like:

		state = state * mul + inc;

The multiplier `mul` is usually fixed; the increment `inc` can be
fixed, but PCG implementations usually allow it to be chosen when the
RNG is seeded.

A bare LCG is a bad RNG. PCG turns an LCG into a good RNG:

  * The LCG state is twice the size of the RNG output
  * The LCG state is permuted to produce the RNG output

[The reference implementation of PCG in C++][pcg-cpp] allows you to
mix and match LCGs and output permutations at a variety of integer
sizes. My implementation provides just one 32-bit RNG and one 64-bit
RNG.

[pcg-cpp]: https://github.com/imneme/pcg-cpp


pcg32
-----

The file `pcg32_xsh_rr.h` contains the preferred 32-bit variant of
PCG. The output permutation's name "XSH RR" is short for "xor shift
rotate right".

In [C++ PCG][pcg-cpp] this variant is called
`pcg_engines::setseq_xsh_rr_64_32` or simply `pcg32` for short.
It is the only variant provided by the [basic C PCG][pcg-basic].

[pcg-basic]: https://github.com/imneme/pcg-c-basic


pcg64
-----

The file `pcg64_dxsm.h` contains the preferred 64-bit variant of PCG.
It is harder to find out about it on [the PCG web site][pcg], though
it is increasingly popular. [Melissa O'Neill describes it as
follows][pcg-dxsm]:

[pcg-dxsm]: https://github.com/imneme/pcg-cpp/commit/871d0494ee9c9a7b7c43f753e3d8ca47c26f8005

> DXSM -- double xor shift multiply
>
> This is a new, more powerful output permutation (added in 2019).  It's
> a more comprehensive scrambling than RXS M, but runs faster on 128-bit
> types.  Although primarily intended for use at large sizes, also works
> at smaller sizes as well.

As well as the DXSM output permutation, this `pcg64` variant uses a
"cheap multiplier", i.e. a 64-bit value half the width of the state,
instead of a 128-bit value the same width as the state. The same
multiplier is used for the LCG and the output permutation.

O'Neill wrote [a longer description of the design of pcg64_dxsm][numpy13635].

[numpy13635]: https://github.com/numpy/numpy/issues/13635#issuecomment-506088698

In [C++ PCG][pcg-cpp] its full name is `pcg_engines::cm_setseq_dxsm_128_64`.
(The C++ PCG typedef `pcg64` still refers to the previously preferred
`xsl_rr` variant.)

In [the Rust `rand_pcg` crate][rust] it is called `Lcg128CmDxsm64`,
i.e. a linear congruential generator with 128 bits of state and a
cheap multiplier, using the DXSM permutation with 64 bits of output.

[rust]: https://rust-random.github.io/rand/rand_pcg/

PCG64DXSM is the default random number generator in NumPy; The NumPy
documentation discusses [how PCG64DXSM improves on the older
PCG64][numpy]. It is proposed to use `pcg64-dxsm` in [the revamp of
Golang's standard library random number API][golang].

[golang]: https://github.com/golang/go/discussions/60751
[numpy]: https://numpy.org/devdocs/reference/random/upgrading-pcg64.html


unbiased bounded random numbers
-------------------------------

Random numbers up to some limit are generated by `pcg32_rand()` and
`pcg64_rand()` using [Daniel Lemire's nearly-divisionless unbiased
rejection sampling algorithm for bounded random
numbers][divisionless]. In this implementation the algorithm is split
into a fast path (which is inlined) and a slow path (a function call,
a division, and the rejection sampling loop).

[divisionless]: https://dotat.at/@/2020-10-29-nearly-divisionless-random-numbers.html

There are also `pcg32_random_float()` and `pcg64_random_double()`
functions that generate random numbers in 0.0 <= ... < 1.0 using
shift-and-multiply.


building
--------

Type `make`.

The files `pcg{32,64}.[ch]` are generated. They are designed to be
self-contained so that you can copy them into your own projects.

The files `pcg32_xsh_rr.c` and `pcg64_dxsm.c` contain the
size-specific algorithms for generating random integer and floating
point values.

The files `pcg.[ch]` contain code that is generic over the bit size:

  * the RNG state type
  * seeding the RNG
  * Lemire's algorithm

The files `pcg{32,64}.def` contain macros to configure the generic
code for 32 bits and 64 bits, respectively.


license
-------

    Written by Tony Finch <dot@dotat.at> in Cambridge.

    Permission is hereby granted to use, copy, modify, and/or
    distribute this software for any purpose with or without fee.

    This software is provided 'as is', without warranty of any kind.
    In no event shall the authors be liable for any damages arising
    from the use of this software.

    SPDX-License-Identifier: 0BSD OR MIT-0
