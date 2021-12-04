#ifndef SRC_EVENTS_
#define SRC_EVENTS_

#include "stdint.h"

#define FLAG_SYSTEM_READY (1UL << 0)

uint32_t events_wait_any(uint32_t flags);
uint32_t events_set(uint32_t flags);
uint32_t events_clear(uint32_t flags);

#endif