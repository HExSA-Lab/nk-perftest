#ifndef __PERF_H__
#define __PERF_H__

#ifdef __NAUTILUS__
	#include <nautilus/libccompat.h>
	//#include <nautilus/pmc.h>
#else
	#include <stdint.h>
	#include <stdio.h>
	//#include <libperf.h>
#endif

typedef struct {
	volatile uint32_t start_lo;
	volatile uint32_t start_hi;
	volatile uint32_t stop_lo;
	volatile uint32_t stop_hi;
	struct libperf_data* pd;
} timer_data_t;

void timer_initialize(timer_data_t* obj);
void timer_print(timer_data_t* obj);
void timer_print_header(char* name);
void timer_finalize(timer_data_t* obj);

static inline void timer_start(timer_data_t* obj) {
	#ifdef __PMC_H__
		reset_all_counters();
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
	{
		unsigned i;
		for (i = 0; i < NUM_PERF_SLOTS; i++) {
			perf_slot_t * slot = &pmc_slots[i];
			if (slot->status == PMC_SLOT_USED) {
				read_event_count(slot->event);
			}
		}
	}
	#endif
}

#endif
