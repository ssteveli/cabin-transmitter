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

}