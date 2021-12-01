#include "mqtt_component.h"

namespace mqtt {

MQTTComponent::MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic) :
    m_friendly_name(friendly_name),
    m_icon(icon),
    m_state_topic(state_topic) {}
    
}