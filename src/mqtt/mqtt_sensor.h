#ifndef SRC_MQTT_SENSOR_
#define SRC_MQTT_SENSOR_

#include "mqtt_component.h"
#include <optional>

namespace mqtt {

class MQTTSensor : public MQTTComponent {
public:
    MQTTSensor(const char* friendly_name, const char* icon, const char* state_topic);
    ~MQTTSensor();
    void set_unit_of_measurement(const std::string &s) { m_unit_of_measurement = s; }
    bool publish_state(float value);
    bool publish_state(uint32_t value);
    bool publish_state(uint8_t value);
    bool publish_state(us_timestamp_t value);

protected:
    const char* unique_id() override;
    const char* component_type() override;
    void set_discovery(JsonDocument& root) override;
    
private:
    std::string m_unit_of_measurement;
};

}
#endif
