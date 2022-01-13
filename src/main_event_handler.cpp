#include "main_event_handler.h"
#include "log.h"
#include "mbed.h"
#include "environment.h"
#include "system.h"
#include "events.h"
#include "mqtt/mqtt_component_discovery.h"
#include "battery_monitor.h"
#include "lte.h"
#include "cloud_config.h"
#include "data_logger.h"

Thread main_event_startup_thread;

void main_event_startup_worker() {
    while (true) {
        log_debug("main event startup worker, waiting on ready flag");
        events_wait_any(FLAG_SYSTEM_READY);

        log_debug("system became ready, calling setups!");

        system_startup();
        mqtt::mqtt_discovery_startup();
    }
}

void main_event_handler_init() {
    events_clear(FLAG_SYSTEM_READY);

    main_event_startup_thread.start(callback(main_event_startup_worker));
    log_info("main_event_handler is ready");
}

void main_event_handler_loop() {
    lte_loop();
    environment_loop();
    system_loop();
    bat_loop();
    data_logger_loop();
}
