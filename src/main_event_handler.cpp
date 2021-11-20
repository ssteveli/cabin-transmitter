#include "main_event_handler.h"
#include <stdint.h>
#include <stdbool.h>
#include "log.h"
#include "mbed.h"

#define MAIN_EVENT_QUEUE_SIZE 20

static main_event_t main_event_current;
static main_event_t main_event_queue[MAIN_EVENT_QUEUE_SIZE] = { };
static uint8_t main_event_queue_index;

void main_event_handler_init() {
    main_event_queue_index = 0;
    memset(main_event_queue, 0x00, sizeof(main_event_queue));
    log_info("main_event_handler is ready");
}

void main_event_handler_loop() {
    if (main_event_dequeue()) {
        switch (main_event_current) {

            default:
                break;
        }
    }

    main_event_current = EVT_IDLE;
}

bool main_event_dequeue() {
    uint8_t i;
    __disable_irq();
    
    if (main_event_queue_index > 0) {
        main_event_current = main_event_queue[0];
        main_event_queue[0] = EVT_IDLE;

        if (main_event_queue_index > 1) {
            for (i = 0; i < main_event_queue_index; i++) {
                main_event_queue[i - 1] = main_event_queue[i];
                main_event_queue[i] = EVT_IDLE;
            }
        }

        main_event_queue_index--;
        __enable_irq();
        return true;
    }

    __enable_irq();
    return false;
}

bool main_event_enqueue(main_event_t event) {
    __disable_irq();

    if (main_event_queue_index == MAIN_EVENT_QUEUE_SIZE) {
        log_info("[MEH]: Queue full, ignoring event");
        return false;
    }

    main_event_queue[main_event_queue_index++] = event;
    __enable_irq();

    return true;
}
