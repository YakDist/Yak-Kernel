#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <yak/ipl.h>

struct cpu_md {
	uint32_t apic_id;
	uint64_t apic_ticks_per_ms;

	ipl_t hw_ipl;
	ipl_t soft_ipl;
};

#ifdef __cplusplus
}
#endif
