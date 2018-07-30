#ifndef __PERF_H__
#define __PERF_H__

#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
	#include <nautilus/pmc.h>
#else
	#include <stdint.h>
	#include <stdio.h>
//#include <libperf.h>
#endif

#ifdef __PMC_H__
	#define NK_PERF_EVENTS 3
#endif

typedef struct {
	volatile uint32_t start_lo;
	volatile uint32_t start_hi;
	volatile uint32_t stop_lo;
	volatile uint32_t stop_hi;

	#ifdef __PMC_H__
		perf_event_t* perf_event[NK_PERF_EVENTS];
		uint64_t perf_event_ctr[NK_PERF_EVENTS];
	#endif

	#ifdef __LIB_LIBPERF_H
		struct libperf_data* pd;
	#endif
} timer_data_t;

void timer_initialize(timer_data_t* obj);
void timer_print(timer_data_t* obj);
void timer_print_header(char* name);
void timer_finalize(timer_data_t* obj);

static inline void timer_start(timer_data_t* obj) {
	#ifdef __PMC_H__
	for(unsigned short i = 0; i < NK_PERF_EVENTS; ++i) {
		nk_pmc_start(obj->perf_event[i]);
	}
	#endif
	// do the bare minimum
	// and be inlined,
	// so this does not affect performance.
	// For best results, timer_data_t should be stack-allocated.
	__asm volatile ("rdtsc\n"
					: "=a" (obj->start_lo), "=d" (obj->start_hi)
					);
}
static inline void timer_stop(timer_data_t* obj) {
	__asm volatile ("rdtsc"
					: "=a" (obj->stop_lo), "=d" (obj->stop_hi)
					);
	#ifdef __PMC_H__
	for(unsigned short i = 0; i < NK_PERF_EVENTS; ++i) {
		obj->perf_event_ctr[i] = nk_pmc_read(obj->perf_event[i]);
		nk_pmc_stop(obj->perf_event[i]);
	}
	#endif
}

#endif
