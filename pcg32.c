#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "pcg32-fanf.h"

pcg32_t
pcg32_randomseed(void) {
	pcg32_t rng;
        int fd = open("/dev/urandom", O_RDONLY);
        if(fd < 0) err(1, "open /dev/urandom");
        ssize_t n = read(fd, &rng, sizeof(rng));
        if(n < (ssize_t)sizeof(rng)) err(1, "read /dev/urandom");
        close(fd);
	/* the LCG increment must be odd */
	rng.inc |= 1;
	return(rng);
}

/*
 * The exact threshold for the resample loop is `(1 << 32) % limit`,
 * calculated using a trick `-limit % limit` to stay within 32 bits.
 * Our caller `pcg32_uniform()` ensures the limit is strictly greater
 * than zero. We sample a fresh random number if we don't get one of
 * `(1 << 32) - residue` values. This is a multiple of the limit, so
 * we are sampling uniformly; and it's the largest multiple that fits
 * in 32 bits, so retries are as rare as possible.
 */
uint32_t
pcg32_uniform_slow(pcg32_t *rng, uint32_t limit, uint64_t num) {
	uint32_t residue = -limit % limit;
        while((uint32_t)(num) < residue)
		num = (uint64_t)pcg32(rng) * (uint64_t)limit;
        return((uint32_t)(num >> 32));
}
