#include "app/timing.h"
#include "database/database.h"
#include "database/my_malloc.h"

#ifdef SMALL
	#define CHUNKS 4096L
	#define COLS 8L
	#define PARAM_MIN 6
	#define PARAM_MAX 9
	#define REPS 1
#else
	#define CHUNKS 4096L
	#define COLS 8L
	#define PARAM_MIN 1
	#define PARAM_MAX 14
	#define REPS 10
#endif
// log2(cols) + log2(chunks) + sizeof(val_t)
#define MODIFIER (12 + 3 + sizeof(val_t))


void test_create_free() {
	volatile uint64_t start, stop, create_time, copy_table_time, copy_row_time, free_time;
	printf("total size (log2 bytes),create,copy table (memcpy),copy rows (individually),free,manipulate a %lu-chunk %lu-col array,title row\n", CHUNKS, COLS);
	for(size_t param = PARAM_MIN; param < PARAM_MAX; ++param) {
		for(size_t reps = 0; reps < REPS; ++reps) {
			size_t chunk_size = 1 << param;
			size_t total_size = chunk_size << MODIFIER;

			#ifdef REPLACE_MALLOC
				// overhead for pointers is added in
				// use -DVERBOSE to tune this
				my_malloc_init(total_size + 2000000);
			#endif

			rdtscll(start);
			col_table_t* table = create_col_table(CHUNKS, chunk_size, COLS, 100);
			rdtscll(stop);
			create_time = stop - start;

			// not counting time to alloc space for copy
			col_table_t* table_copy = create_col_table_like(table);

			rdtscll(start);
			copy_col_table_noalloc(table, table_copy);
			rdtscll(stop);
			copy_table_time = stop - start;

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
			free_col_table(table);
			rdtscll(stop);
			free_time = stop - start;

			free_col_table(table_copy);

			#ifdef REPLACE_MALLOC
				my_malloc_deinit();
			#endif

			printf("%lu,%lu,%lu,%lu,%lu\n", param + MODIFIER, create_time, copy_table_time, copy_row_time, free_time);
		}
	}
}
