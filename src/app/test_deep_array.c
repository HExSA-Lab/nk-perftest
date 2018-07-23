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
	#define LOG_DIM1_MIN 20
	#define LOG_DIM1_MAX 21
	#define LOG_DIM2_MIN 10
	#define LOG_DIM2_MAX 23
#else
	#define REPS 10
	#define LOG_DIM1_MIN 12
	#define LOG_DIM1_MAX 13
	#define LOG_DIM2_MIN 10
	#define LOG_DIM2_MAX 19
#endif

typedef uint8_t mval_t;

static inline mval_t** deep_array_create(size_t dim1, size_t dim2) {
	void* chunk = malloc(dim1 * sizeof(mval_t*) + dim1 * dim2 * sizeof(mval_t));

	mval_t** ret = chunk;
	chunk += dim1 * sizeof(mval_t*);
	for(size_t i = 0; i < dim1; ++i) {
		ret[i] = chunk;
		chunk += dim2 * sizeof(mval_t);
	}
	return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static inline void deep_array_destroy(mval_t** arr, size_t dim1) {
#pragma GCC diagnostic pop
	free(arr);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static inline void deep_array_get1(mval_t** arr, size_t dim1, size_t dim2) {
	for(size_t i = 0; i < dim1; ++i) {
		for(size_t j = 0; j < dim2; ++j) {
			volatile mval_t a = arr[i][j];
		}
	}
}

static inline void deep_array_get2(mval_t** arr, size_t dim1, size_t dim2) {
	for(size_t j = 0; j < dim2; ++j) {
		for(size_t i = 0; i < dim1; ++i) {
			volatile mval_t a = arr[i][j];
		}
	}
}
#pragma GCC diagnostic pop

static inline void deep_array_set1(mval_t** arr, size_t dim1, size_t dim2) {
	for(size_t i = 0; i < dim1; ++i) {
		for(size_t j = 0; j < dim2; ++j) {
			arr[i][j] = 0;
		}
	}
}

static inline void deep_array_set2(mval_t** arr, size_t dim1, size_t dim2) {
	for(size_t j = 0; j < dim2; ++j) {
		for(size_t i = 0; i < dim1; ++i) {
			arr[i][j] = 0;
		}
	}
}

/*
static inline void deep_array_copy1(mval_t** arr1, mval_t** arr2, size_t dim1, size_t dim2) {
	for(size_t i = 0; i < dim1; ++i) {
		for(size_t j = 0; j < dim2; ++j) {
			arr2[i][j] = arr1[i][j];
		}
	}
}

static inline void deep_array_copy2(mval_t** arr1, mval_t** arr2, size_t dim1, size_t dim2) {
	for(size_t j = 0; j < dim2; ++j) {
		for(size_t i = 0; i < dim1; ++i) {
			arr2[i][j] = arr1[i][j];
		}
	}
}
*/
void test_deep_array() {
	printf("array size (log2 bytes),noop,create,get1,get2,set1,set2,destroy,deep array,title row\n");
	for(size_t log_dim1 = LOG_DIM1_MIN; log_dim1 < LOG_DIM1_MAX; ++log_dim1) {
		size_t dim1 = 1 << log_dim1;

		for(size_t log_dim2 = LOG_DIM2_MIN; log_dim2 < LOG_DIM2_MAX; ++log_dim2) {
			size_t dim2 = 1 << log_dim2;

			for(size_t reps = 0; reps < REPS; ++reps) {
				uint64_t null_time = 0, create_time = 0, destroy_time = 0, get1_time = 0, get2_time = 0, set1_time = 0, set2_time = 0;
				volatile uint64_t start = 0, stop = 0;

				rdtscll(start);
				rdtscll(stop);
				null_time = stop - start;

				rdtscll(start);
				mval_t** arr1 = deep_array_create(dim1, dim2);
				mval_t** arr2 = deep_array_create(dim1, dim2);
				rdtscll(stop);
				create_time = stop - start;

				rdtscll(start);
				deep_array_get1(arr1, dim1, dim2);
				rdtscll(stop);
				get1_time = stop - start;

				rdtscll(start);
				deep_array_get2(arr1, dim1, dim2);
				rdtscll(stop);
				get2_time = stop - start;

				rdtscll(start);
				deep_array_set1(arr1, dim1, dim2);
				rdtscll(stop);
				set1_time = stop - start;

				rdtscll(start);
				deep_array_set2(arr1, dim1, dim2);
				rdtscll(stop);
				set2_time = stop - start;

				rdtscll(start);
				deep_array_destroy(arr1, dim1);
				deep_array_destroy(arr2, dim1);
				rdtscll(stop);
				destroy_time = stop - start;

				printf("%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", log_dim1 + log_dim2, null_time, create_time, get1_time, get2_time, set1_time, set2_time, destroy_time);
			}
		}
	}
}
