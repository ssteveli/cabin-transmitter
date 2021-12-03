#ifndef SRC_MQTT_COMPONENT_
#define SRC_MQTT_COMPONENT_

#include "mbed.h"

namespace mqtt {

class MQTTComponent {
public:
    MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic);
    virtual ~MQTTComponent();

    virtual bool send_discovery() = 0;
    bool publish_state(const char* format, ...);
    bool publish_state(const char* format, mbed::Callback<void(bool)> _cb, ...);
    void set_timeout(int timeout) { m_timeout = timeout; }
    int get_timeout(int timeout) { return m_timeout; }

protected:
    virtual const char* unique_id() = 0;
    virtual const char* component_type() = 0;
    
    const char* friendly_name() {
        return m_friendly_name;
    }

    const char* icon() {
        return m_icon;
    }

    const char* state_topic() {
        return m_state_topic;
    }

private:
    const char* m_friendly_name;
    const char* m_icon;
    const char* m_state_topic;
    int m_timeout = 1000;
};

}

#endif
