#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <assert.h>
	#include <stdbool.h>
#endif

#include "perf/perf.h"
#include "database/rand.h"
#include "database/database.h"
#include "database/operators.h"
#include "database/my_malloc.h"
#include "database/rand.h"

typedef unsigned long ulong;

#ifdef SMALL
	#define log_num_chunks 8
	#define log_num_cols_min 6
	#define log_num_cols_max (log_num_cols_min + 1)
	#define log_chunk_size_min  8
	#define log_chunk_size_max  (log_chunk_size_min + 1)
	#define domain_size 100
	#define REPS       1
	#define RAND_SEED 0
#else
	#define log_num_chunks 8
	#define log_num_cols_min 0
	#define log_num_cols_max 7
	#define log_chunk_size_min  8
	#define log_chunk_size_max  14
	#define domain_size 100
	#define REPS       5
	#define RAND_SEED 0
#endif

#define LOG_SIZEOF_VAL_T 2

// This is because I allocate
//   - the test table,
//   - a copy of it,
//   - an output table (inside of countingsort),
//   - a bit_vector whose size is the table
// Therefore I need 3 times the size of one table.
// Furthermore, sort and check_sorted both need to allocate arrays for other purposes.
// The LOG_TOTAL_SIZE is approximate and less than the real table size,
// because it only counts allocating the data-chunks, not pointers to those data-chunks.
#define TOTAL_SIZE_EXTRA_FACTOR 5.5
#define TOTAL_SIZE_EXTRA 1000000

void test_db() {
	assert(1 << LOG_SIZEOF_VAL_T == sizeof(val_t));

	ulong num_chunks = 1 << log_num_chunks;
	rand_seed(RAND_SEED);

	timer_data_t timer;
	timer_initialize(&timer);

	printf("chunk size,cols,");
	timer_print_header("copy table (memcpy)");
	timer_print_header("copy table (individually)");
	timer_print_header("iterate");
	timer_print_header("sort (column-oriented)");
	timer_print_header("sort (row-oriented)");
	printf("on a %lu-chunk array,title row\n", num_chunks);

	for(ulong log_chunk_size = log_chunk_size_min; log_chunk_size < log_chunk_size_max; ++log_chunk_size) {
		for(ulong log_num_cols = log_num_cols_min; log_num_cols < log_num_cols_max; ++log_num_cols) {

			for(ulong reps = 0; reps < REPS; ++reps) {

				ulong chunk_size = 1 << log_chunk_size;
				ulong log_total_size = log_num_chunks + log_chunk_size + log_num_cols + LOG_SIZEOF_VAL_T;
				ulong total_size = 1 << log_total_size;
				ulong num_cols = 1 << log_num_cols;

				my_malloc_init(((ulong) (total_size * TOTAL_SIZE_EXTRA_FACTOR)) + TOTAL_SIZE_EXTRA);

				col_table_t* table = create_col_table(num_chunks, chunk_size, num_cols, domain_size);
				col_table_t* table_copy = create_col_table_like(table);

				printf("%lu,%lu,", log_chunk_size, log_num_cols);

				// meaningless when we have -DREPLACE_MALLOC
				//timer_start(&timer);
				//col_table_t* table = create_col_table(num_chunks, chunk_size, num_cols, domain_size);
				//col_table_t* table_copy = create_col_table_like(table);
				//timer_stop_print(&timer);

				timer_start(&timer);
				copy_col_table_noalloc(table, table_copy);
				timer_stop_print(&timer);

				timer_start(&timer);
				for(ulong chunk_no = 0; chunk_no < table->num_chunks; ++chunk_no) {
					table_chunk_t *in_chunk  = table     ->chunks[chunk_no];
					table_chunk_t *out_chunk = table_copy->chunks[chunk_no];
					for(ulong offset = 0; offset < in_chunk->columns[0]->chunk_size; ++offset) {
						// copy_row(in_chunk, offset, out_chunk, offset, COLS);
						for(ulong column = 0; column < num_cols; ++column) {
							in_chunk ->columns[column]->data[offset] =
								out_chunk->columns[column]->data[offset];
						}
					}
				}
				timer_stop_print(&timer);

				timer_start(&timer);
				#pragma GCC diagnostic push
				#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
				volatile val_t val;
				#pragma GCC diagnostic pop

				for(ulong chunk_no = 0; chunk_no < table->num_chunks; ++chunk_no) {
					table_chunk_t *chunk  = table->chunks[chunk_no];
					for(ulong offset = 0; offset < chunk->columns[0]->chunk_size; ++offset) {
						for(ulong column = 0; column < num_cols; ++column) {
							val = chunk->columns[column]->data[offset];
						}
					}
				}
				timer_stop_print(&timer);

				timer_start(&timer);
				// note that this also counts the time to alloc a new table
				ulong SORT_COL = num_cols / 2;
				table = countingmergesort(table, SORT_COL, domain_size);
				timer_stop_print(&timer);
				bool sorted = check_sorted(table, SORT_COL, domain_size, table_copy);
				if(!sorted) {
					printf("table not sorted;\n");
					exit(1);
				}

				timer_start(&timer);
				// note that this also counts the time to alloc a new table
				table_copy = countingmergesort2(table_copy, SORT_COL, domain_size);
				timer_stop_print(&timer);
				bool sorted2 = check_sorted(table_copy, SORT_COL, domain_size, table);
				if(!sorted2) {
					printf("table_copy not sorted;\n");
					exit(1);
				}

				//timer_start(&timer);
				//free_col_table(table);
				//free_col_table(table_copy);
				//timer_stop_print(&timer);

				printf("\n");

				free_col_table(table);
				free_col_table(table_copy);

#ifdef VERBOSE
#ifdef REPLACE_MALLOC
				my_malloc_print();
#endif
#endif

				// this is noop if REPLACE MALLOC is undefined
				my_malloc_deinit();

			}
		}
	}
}
