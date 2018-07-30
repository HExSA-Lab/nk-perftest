#include "perf/perf.h"

#ifdef __PMC_H__
	char* perf_event_hdr[NK_PERF_EVENTS] = {
		"dcache",
		"icache",
		"tlb miss",
	};
	unsigned int perf_event_idx[NK_PERF_EVENTS] = {
		0,
		1,
		3,
	};
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void timer_initialize(timer_data_t* obj) {
#pragma GCC diagnostic pop

	#ifdef __PMC_H__
	for(unsigned int i = 0; i < NK_PERF_EVENTS; ++i) {
		obj->perf_event[i] = nk_pmc_create(perf_event_idx[i]);
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

	#ifdef __PMC_H__
	for(unsigned int i = 0; i < NK_PERF_EVENTS; ++i) {
		printf("%lu,", obj->perf_event_ctr[i]);
	}
	#endif
}


void timer_print_header(char* name) {
	printf("%s time,", name);
	#ifdef __PMC_H__
	for(unsigned int i = 0; i < NK_PERF_EVENTS; ++i) {
		printf("%s ,", name);
	}
	#endif
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void timer_finalize(timer_data_t* obj) {
#pragma GCC diagnostic pop
	#ifdef __PMC_H__
	for(unsigned short i = 0; i < NK_PERF_EVENTS; ++i) {
		nk_pmc_destroy(obj->perf_event[i]);
	}
	#endif

	#ifdef __LIB_LIBPERF_H
	{
		libperf_finalize(pd, 0);
	}
	#endif
}
