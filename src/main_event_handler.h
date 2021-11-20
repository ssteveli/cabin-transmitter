#pragma once

#include <stdbool.h>

typedef enum {
    EVT_IDLE,
    EVT_LTE
} main_event_t;

void main_event_handler_init();
void main_event_handler_loop();
bool main_event_enqueue(main_event_t event);
bool main_event_dequeue();
