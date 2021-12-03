#include "mqtt_component.h"
#include "mbed.h"
#include "lte.h"
#include "ArduinoJson.h"

namespace mqtt {

MQTTComponent::MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic) :
    m_friendly_name(friendly_name),
    m_icon(icon),
    m_state_topic(state_topic) {}

MQTTComponent::~MQTTComponent() {}

bool MQTTComponent::send_discovery() {
    std::string discovery_topic = "homeassistant/" + std::string(this->component_type()) + "/cabin/" + this->m_friendly_name + "/config";
    
    DynamicJsonDocument root(1024);
    root["name"] = m_friendly_name;
    root["state_topic"] = m_state_topic;
    root["icon"] = m_icon;

    char buf[1024];
    serializeJson(root, buf);
    return true;
}

bool MQTTComponent::publish_state(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
    bool result = publish_state(format, NULL, vl);
    va_end(vl);

    return result;
}

bool MQTTComponent::publish_state(const char* format, mbed::Callback<void(bool)> _cb, ...) {
    va_list vl;
    va_start(vl, _cb);
    bool result = lte_publish(m_state_topic, format, _cb, m_timeout, vl);
    va_end(vl);

    return result;
}

}
