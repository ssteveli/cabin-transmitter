#include "mqtt_sensor.h"

namespace mqtt {

MQTTSensor::MQTTSensor(const char* friendly_name, const char* icon, const char* state_topic) :
    MQTTComponent(friendly_name, icon, state_topic) {}

MQTTSensor::~MQTTSensor() {}

bool MQTTSensor::send_discovery() {
    return false;
}

const char* MQTTSensor::unique_id() {
    return "TBD";
}

}