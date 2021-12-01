#ifndef SRC_MQTT_SENSOR_
#define SRC_MQTT_SENSOR_

#include "mqtt_component.h"

namespace mqtt {

class MQTTSensor : MQTTComponent {
public:
    MQTTSensor(const char* friendly_name, const char* icon, const char* state_topic);    
    bool send_discovery() override;
    bool publish_state() override;
};

}
#endif
