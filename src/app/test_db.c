#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <assert.h>
	#include <stdbool.h>
#endif
#include "app/timing.h"
#include "database/database.h"
#include "database/operators.h"
#include "database/my_malloc.h"

#ifdef SMALL
	#define LOG_CHUNKS 6
	#define PARAM_MIN  16
	#define PARAM_MAX  18
	#define REPS       1
#else
	#define LOG_CHUNKS 12
	#define PARAM_MIN  6
	#define PARAM_MAX  12
	#define REPS       10
#endif

#define LOG_COLS       3
#define LOG_SIZEOF_VAL_T 2
#define LOG_TOTAL_SIZE (LOG_CHUNKS + LOG_COLS + LOG_SIZEOF_VAL_T)

#define DOMAIN_SIZE 100
#define SORT_COL 3

#define CHUNKS         (1L << LOG_CHUNKS)
#define COLS           (1L << LOG_COLS)
#define TOTAL_SIZE     (1L << LOG_TOTAL_SIZE)

// This is because I allocate (1) the test table, (2) a copy of it, and (3) an output table (inside of countingsort)
// Therefore I need 3 times the size of one table.
// Furthermore, sort and check_sorted both need to allocate arrays for other purposes.
// The LOG_TOTAL_SIZE is approximate and less than the real table size,
// because it only counts allocating the data-chunks, not pointers to those data-chunks.
#define TOTAL_SIZE_EXTRA_FACTOR 4
#define TOTAL_SIZE_EXTRA 0

void test_db() {
	volatile uint64_t start, stop;
	uint64_t create_time = 0, copy_table_time = 0, copy_row_time = 0, iterate_time = 0, free_time = 0, sort_time = 0;
	printf("total size (log2 bytes),create,copy table (memcpy),copy rows (individually),sort,iterate,free,manipulate a %lu-chunk %lu-col array,title row\n", CHUNKS, COLS);
	for(size_t log_chunk_size = PARAM_MIN; log_chunk_size < PARAM_MAX; ++log_chunk_size) {
		for(size_t reps = 0; reps < REPS; ++reps) {
			size_t chunk_size = 1 << log_chunk_size;

			assert(1 << LOG_SIZEOF_VAL_T == sizeof(val_t));

			#ifdef REPLACE_MALLOC
				my_malloc_init(TOTAL_SIZE * chunk_size * TOTAL_SIZE_EXTRA_FACTOR + TOTAL_SIZE_EXTRA);
			#endif

			rdtscll(start);
			col_table_t* table = create_col_table(CHUNKS, chunk_size, COLS, DOMAIN_SIZE);
			col_table_t* table_copy = create_col_table_like(table);
			rdtscll(stop);
			create_time = stop - start;

			rdtscll(start);
			copy_col_table_noalloc(table, table_copy);
			rdtscll(stop);
			copy_table_time = stop - start;

			rdtscll(start);
			for(size_t chunk_no = 0; chunk_no < table->num_chunks; ++chunk_no) {
				table_chunk_t *in_chunk  = table     ->chunks[chunk_no];
				table_chunk_t *out_chunk = table_copy->chunks[chunk_no];
				for(size_t offset = 0; offset < in_chunk->columns[0]->chunk_size; ++offset) {
					// copy_row(in_chunk, offset, out_chunk, offset, COLS);
					for(size_t column = 0; column < COLS; ++column) {
						in_chunk ->columns[column]->data[offset] =
						 out_chunk->columns[column]->data[offset];
					}
				}
			}
			rdtscll(stop);
			copy_row_time = stop - start;

			rdtscll(start);
			// note that this also counts the time to alloc a new table
			table = countingmergesort(table, SORT_COL, DOMAIN_SIZE);
			rdtscll(stop);
			sort_time = stop - start;

			rdtscll(start);
			volatile val_t val;
			for(size_t chunk_no = 0; chunk_no < table->num_chunks; ++chunk_no) {
				table_chunk_t *chunk  = table->chunks[chunk_no];
				for(size_t offset = 0; offset < in_chunk->columns[0]->chunk_size; ++offset) {
					// copy_row(in_chunk, offset, out_chunk, offset, COLS);
					for(size_t column = 0; column < COLS; ++column) {
						val = chunk->columns[column]->data[offset];
					}
				}
			}
			rdtscll(stop);
			iterate_time = stop - start;

			bool sorted = check_sorted(table, SORT_COL, DOMAIN_SIZE, table_copy);
			if(!sorted) {
				printf("Not sorted!\n");
			}

			rdtscll(start);
			free_col_table(table);
			free_col_table(table_copy);
			rdtscll(stop);
			free_time = stop - start;

			#ifdef REPLACE_MALLOC
				#ifdef VERBOSE
					my_malloc_print();
				#endif
				my_malloc_deinit();
			#endif

				printf("%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", log_chunk_size + LOG_TOTAL_SIZE, create_time, copy_table_time, copy_row_time, sort_time, iterate_time, free_time);
		}
	}
}
