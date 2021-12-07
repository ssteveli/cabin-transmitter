#include "mqtt_component_discovery.h"
#include "../events.h"
#include <vector>
#include "log.h"
#include "mbed.h"

namespace mqtt {

std::vector<MQTTComponent*> mqtt_components;

void mqtt_discovery_startup() {
    for (auto &component : mqtt_components) {
        log_debug("registering component: %s", component->get_friendly_name());
        component->send_discovery();
    }
}

void mqtt_register_component(MQTTComponent *component) {
    mqtt_components.push_back(component);
}

void mqtt_component_discovery_init() {
    mqtt_components.clear();
}

}