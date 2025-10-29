// SPDX-License-Identifier: 0BSD OR MIT-0

#ifdef __APPLE__

/*
 * need a nonstandard API to get nanosecond resolution, sigh
 */

#include <mach/mach_time.h>

static uint64_t
nanotime(void) {
	static mach_timebase_info_data_t scale;
	if (scale.denom == 0) {
		kern_return_t status = mach_timebase_info(&scale);
		assert(status == KERN_SUCCESS);
	}
	__sync_synchronize();
	return (mach_absolute_time() * scale.numer / scale.denom);
}

#else

#include <time.h>

#define NS_PER_S (1000 * 1000 * 1000)

static uint64_t
nanotime(void) {
	struct timespec tv;
	assert(clock_gettime(CLOCK_MONOTONIC, &tv) == 0);
	__sync_synchronize();
	return ((uint64_t)tv.tv_sec * NS_PER_S + (uint64_t)tv.tv_nsec);
}

#endif
