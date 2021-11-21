#include "mbed.h"
#include "main_event_handler.h"
#include "lte.h"
#include "log.h"
#include "config.h"
#include "environment.h"
#include "rtc.h"
#include "system.h"

static void main_init();

void handle(bool result) {
    log_debug("result: %d on ctx %p", result, ThisThread::get_id());
}

int main() {
    main_init();

    log_debug("starting main loop on ctx %p", ThisThread::get_id());
    while(1) {
    }
}

static void main_init() {
    log_init();
    config_init();
    rtc_start();
    main_event_handler_init();
    lte_init();
    system_init();
    environment_init();
}