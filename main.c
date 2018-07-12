#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
#endif

size_t param;

#include "test.c"

#define rdtscll(val)                                    \
    do {                                                \
        uint32_t hi, lo;                                \
        __asm volatile("rdtsc" : "=a" (lo), "=d" (hi)); \
        val = ((uint64_t)hi << 32) | lo;                \
    } while (0)

#define REPS 10
void app_main() {
	volatile uint64_t start, stop;
	printf(TEST_NAME "\n");
	printf(PARAM_DESCR ",time (cycles)\n");
	for(size_t i = 0; i < REPS; ++i) {
		for(param = PARAM_MIN; param < PARAM_MAX; ++param) {
			test_setup();
			rdtscll(start);
			test_run();
			rdtscll(stop);
			printf("%lu,%lu\n", param, stop - start);
			test_teardown();
		}
	}
}

#ifndef __NAUTILUS__
int main() {
	app_main();
	return 0;
}
#endif
