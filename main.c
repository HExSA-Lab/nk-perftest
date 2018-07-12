#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
#endif

#define rdtscll(val)                                    \
    do {                                                \
        uint32_t hi, lo;                                \
        __asm volatile("rdtsc" : "=a" (lo), "=d" (hi)); \
        val = ((uint64_t)hi << 32) | lo;                \
    } while (0)

#define SIZE 10*1000*1000
#define REPS 10

typedef uint64_t val_t;

void app_main() {
	for(size_t i = 0; i < REPS; ++i) {
		val_t* array = malloc(SIZE * sizeof(val_t));
		val_t tmp;
		volatile uint64_t start, stop;

		rdtscll(start);
		for(size_t i = 0, j = SIZE - 1; i < SIZE; ++i, --j) {
			tmp      = array[i];
			array[i] = array[j];
			array[j] = tmp;
		}
		rdtscll(stop);

		printf("time: exp, %lu\n", stop - start);

		free(array);
	}
}

#ifndef __NAUTILUS__
int main() {
	app_main();
	return 0;
}
#endif
