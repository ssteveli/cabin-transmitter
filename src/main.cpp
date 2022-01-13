#include "mbed.h"
#include "main_event_handler.h"
#include "lte.h"
#include "log.h"
#include "local_config.h"
#include "environment.h"
#include "rtc.h"
#include "system.h"
#include "mqtt/mqtt_component_discovery.h"
#include "battery_monitor.h"
#include "cloud_config.h"
#include "data_logger.h"

static void main_init();

int main() {
    main_init();

    rtc_time_t t;
    if (rtc_read_time(&t) == 0) {
        log_debug("############################################################");
        log_debug("time: %02d/%02d/%d %02d:%02d:%02d", t.month, t.date, t.year, t.hours, t.minute, t.second);
        log_debug("############################################################");
    } else {
        log_debug("rtc failed");
    }


    log_debug("starting main loop on ctx %p", ThisThread::get_id());
    while(1) {
        main_event_handler_loop();
        
        ThisThread::yield();
    }
}

static void main_init() {
    log_init();
    local_config_init();
    cloud_config_init();
    rtc_start();
    mqtt::mqtt_component_discovery_init();
    main_event_handler_init();
    lte_init();
    system_init();
    environment_init();
    bat_init();
    data_logger_init();
}