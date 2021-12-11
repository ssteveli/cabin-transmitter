#ifndef SRC_MQTT_BINARY_SENSOR_
#define SRC_MQTT_BINARY_SENSOR_

#include "mqtt_component.h"

namespace mqtt {

class MQTTBinarySensor : public MQTTComponent {
public:
    MQTTBinarySensor(const char* friendly_name, const char* icon, const char* state_topic);
    ~MQTTBinarySensor();
    bool publish_state(bool state, mbed::Callback<void(bool)> _cb);

protected:
    const char* unique_id() override;
    const char* component_type() override;
    void set_discovery(JsonDocument& root) override;
};

}
#endif
