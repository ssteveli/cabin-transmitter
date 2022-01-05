#include "main_event_handler.h"
#include "log.h"
#include "mbed.h"
#include "environment.h"
#include "system.h"
#include "events.h"
#include "mqtt/mqtt_component_discovery.h"
#include "battery_monitor.h"
#include "lte.h"
#include <vector>

Thread main_event_startup_thread;

std::vector<service_t> enabled_services;

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

void main_event_enable(service_t service, bool enabled) {
    if (enabled) {
        enabled_services.push_back(service);
    } else {
        std::remove(enabled_services.begin(), enabled_services.end(), service);
    }
}

void main_event_handler_loop() {
    // process any inbound async messages
    lte_oob_loop();

    for (const service_t service : enabled_services) {
        switch (service) {
            case ENVIRONMENT:
                environment_loop();
                break;
            
            case SYSTEM:
                system_loop();
                break;
            
            case BATTERY:
                bat_loop();
                break;

            case LTE:
                lte_loop();
                break;
                
            default:
                break;
        }
    }
}
