#include "perf/perf.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void timer_initialize(timer_data_t* obj) {
#pragma GCC diagnostic pop
	#ifdef __PMC_H__
	{
		pmc_init();
		enable_all_events();
	}
	#endif
	#ifdef __LIB_LIBPERF_H
	{
		obj->pd = libperf_initialize(-1,-1);
	}
	#endif
}

void timer_print(timer_data_t* obj) {
	uint64_t start_cycles = ((uint64_t)obj->start_hi << 32) | obj->start_lo;
	uint64_t stop_cycles = ((uint64_t)obj->stop_hi << 32) | obj->stop_lo;
	uint64_t elapsed_cycles = stop_cycles - start_cycles;
	printf("%lu,", elapsed_cycles);
}


void timer_print_header(char* name) {
	printf("%s time,", name);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void timer_finalize(timer_data_t* obj) {
#pragma GCC diagnostic pop
	#ifdef __LIB_LIBPERF_H
		libperf_finalize(pd, 0);
	#endif
}
