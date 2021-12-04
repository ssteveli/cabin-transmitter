#include "events.h"
#include "mbed.h"

EventFlags event_flags("cabin");

uint32_t events_wait_any(uint32_t flags) {
    return event_flags.wait_any(flags);
}

uint32_t events_set(uint32_t flags) {
    return event_flags.set(flags);
}

uint32_t events_clear(uint32_t flags) {
    return event_flags.clear(flags);
}