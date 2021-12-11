#include "mqtt_binary_sensor.h"

namespace mqtt {

MQTTBinarySensor::MQTTBinarySensor(const char* friendly_name, const char* icon, const char* state_topic) :
    MQTTComponent(friendly_name, icon, state_topic) {}

MQTTBinarySensor::~MQTTBinarySensor() {}

const char* MQTTBinarySensor::unique_id() {
    return get_friendly_name();
}

const char* MQTTBinarySensor::component_type(){
    return "binary_sensor";
}

void MQTTBinarySensor::set_discovery(JsonDocument& root) {
}

bool MQTTBinarySensor::publish_state(bool state, mbed::Callback<void(bool)> _cb) {
    const std::string state_str = state ? "ON" : "OFF";
    return publish(state_str.c_str(), _cb);
}

}