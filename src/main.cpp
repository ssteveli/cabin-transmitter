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

    rtc_time_t t;
    if (rtc_read_time(&t) == 0) {
        log_debug("time: %d/%d/%d %d:%d:%d", t.month, t.day, t.year, t.hours, t.minute, t.second);
    } else {
        log_debug("rtc failed");
    }


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