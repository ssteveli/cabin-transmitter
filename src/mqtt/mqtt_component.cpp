#include "mqtt_component.h"
#include "mbed.h"
#include "lte.h"
#include "ArduinoJson.h"
#include "log.h"

namespace mqtt {

MQTTComponent::MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic) :
    m_friendly_name(friendly_name),
    m_icon(icon),
    m_state_topic(state_topic) {}

MQTTComponent::~MQTTComponent() {}

void find_and_replace(std::string &data, std::string to_search, std::string replace_with) {
    size_t pos = data.find(to_search);
    while (pos != std::string::npos) {
        data.replace(pos, to_search.size(), replace_with);
        pos = data.find(to_search, pos + replace_with.size());
    }
}

bool MQTTComponent::send_discovery() {
    std::string discovery_topic = "homeassistant/" + std::string(this->component_type()) + "/cabin/" + this->m_friendly_name + "/config";
    
    DynamicJsonDocument root(1024);
    root["name"] = m_friendly_name;
    root["state_topic"] = m_state_topic;
    root["icon"] = m_icon;
    root["unique_id"] = unique_id();
    set_discovery(root);

    std::string buf = "";
    serializeJson(root, buf);
    log_debug("discovery json: %s", buf.c_str());
   
    lte_publish(discovery_topic.c_str(), buf.c_str(), NULL, 5000, true);

    return true;
}

bool MQTTComponent::publish_state(const char* format, ...) {
    va_list vl;
    va_start(vl, format);
    bool result = vpublish_state(format, NULL, vl);
    va_end(vl);

    return result;
}

bool MQTTComponent::publish_state(const char* format, mbed::Callback<void(bool)> _cb, ...) {
    va_list vl;
    va_start(vl, _cb);
    bool result = lte_vpublish(m_state_topic, format, _cb, m_timeout, m_retain, vl);
    va_end(vl);
    
    return result;
}

bool MQTTComponent::vpublish_state(const char* format, va_list args) {
    return vpublish_state(format, NULL, args);
}

bool MQTTComponent::vpublish_state(const char* format, mbed::Callback<void(bool)> _cb, va_list args) {
    return lte_vpublish(m_state_topic, format, _cb, m_timeout, m_retain, args);
}

} // end namespace
