#define TEST_NAME "reverse"
#define PARAM_DESCR "array size (bytes log2)"
#define PARAM_MIN 1
#define PARAM_MAX 31

typedef uint8_t val_t;

val_t* array;
size_t size;
static inline void test_setup() {
	size = 1 << param;
	array = malloc(size * sizeof(val_t));
}

static inline void test_run() {
	val_t tmp;
	for(size_t i = 0, j = size - 1; i < size; ++i, --j) {
		tmp      = array[i];
		array[i] = array[j];
		array[j] = tmp;
	}
}

static inline void test_teardown() {
	free(array);
	array = NULL;
}
