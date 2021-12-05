#ifndef SRC_COMPONENT_DISCOVERY_
#define SRC_COMPONENT_DISCOVERY_

#include "mqtt_component.h"

namespace mqtt {

void mqtt_register_component(MQTTComponent *component);
void mqtt_component_discovery_init();
void mqtt_discovery_startup();

}

#endif