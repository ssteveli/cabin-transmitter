#ifndef SRC_MQTT_SENSOR_
#define SRC_MQTT_SENSOR_

#include "mqtt_component.h"

namespace mqtt {

class MQTTSensor : public MQTTComponent {
public:
    MQTTSensor(const char* friendly_name, const char* icon, const char* state_topic);
    ~MQTTSensor();

    bool send_discovery() override;

protected:
    const char* unique_id() override;
};

}
#endif
