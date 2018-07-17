#include "app/timing.h"
#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <stdint.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

#ifdef SMALL
	#define REPS 1
	#define PARAM_MIN 20
	#define PARAM_MAX 23
#else
	#define REPS 10
	#define PARAM_MIN 8
	#define PARAM_MAX 31
#endif

typedef uint8_t mval_t;

void test_malloc_free() {
	printf("array size (log2 bytes),malloc,memcpy,reverse,free,,title row\n");
	for(size_t param = PARAM_MIN; param < PARAM_MAX; ++param) {
		for(size_t reps = 0; reps < REPS; ++reps) {
			size_t size = 1 << param;
			volatile uint64_t start, stop;
			uint64_t malloc_time = 0, free_time = 0, memcpy_time = 0, reverse_time = 0;

			rdtscll(start);
			mval_t* array = malloc(size * sizeof(mval_t));
			rdtscll(stop);
			malloc_time = stop - start;

			mval_t* array_copy = malloc(size * sizeof(mval_t));
			rdtscll(start);
			memcpy(array_copy, array, size * sizeof(mval_t));
			rdtscll(stop);
			memcpy_time = stop - start;
			free(array_copy);

			rdtscll(start);
			mval_t tmp;
			for(size_t i = 0, j = size - 1; i < size; ++i, --j) {
				tmp      = array[i];
				array[i] = array[j];
				array[j] = tmp;
			}
			rdtscll(stop);
			reverse_time = stop - start;

			rdtscll(start);
			free(array);
			rdtscll(stop);
			free_time = stop - start;

			printf("%lu,%lu,%lu,%lu,%lu\n", param, malloc_time, memcpy_time, reverse_time, free_time);

		}
	}
}
