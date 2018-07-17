#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
#else
	#include <stdio.h>
	#include <stdint.h>
	#include <stdlib.h>
#endif

#include "app/test_create_free.h"
#include "app/test_malloc_free.h"

void test() {
	test_malloc_free();
	test_create_free();
}

#ifdef __NAUTILUS__
void app_main() {
	printf("begin_data\n");
	test();
	printf("end_data\n");
}
#else
int main() {
	test();
	return 0;
}
#endif
