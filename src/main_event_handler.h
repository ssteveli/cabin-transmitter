#pragma once

#include <stdbool.h>

typedef enum {
    EVT_IDLE,
    EVT_LTE
} main_event_t;

typedef enum {
    ENVIRONMENT,
    BATTERY,
    SYSTEM,
    LTE
} service_t;

void main_event_enable(service_t service, bool enabled);
void main_event_handler_init();
void main_event_handler_loop();
bool main_event_enqueue(main_event_t event);
bool main_event_dequeue();
