#include "mqtt_sensor.h"

namespace mqtt {

MQTTSensor::MQTTSensor(const char* friendly_name, const char* icon, const char* state_topic) :
    MQTTComponent(friendly_name, icon, state_topic) {}

MQTTSensor::~MQTTSensor() {}

const char* MQTTSensor::unique_id() {
    return get_friendly_name();
}

const char* MQTTSensor::component_type(){
    return "sensor";
}

void MQTTSensor::set_discovery(JsonDocument& root) {
    if (!m_unit_of_measurement.empty()) {
        root["unit_of_measurement"] = m_unit_of_measurement;
    }
}

bool MQTTSensor::publish_state(float value) {
    return publish("%0.1f", value);
}

bool MQTTSensor::publish_state(uint32_t value) {
    return publish("%ld", value);
}

bool MQTTSensor::publish_state(uint8_t value) {
    return publish("%d", value);
}

bool MQTTSensor::publish_state(us_timestamp_t value) {
    return publish("%ld", value);
}

}