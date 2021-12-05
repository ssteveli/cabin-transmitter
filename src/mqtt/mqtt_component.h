#ifndef SRC_MQTT_COMPONENT_
#define SRC_MQTT_COMPONENT_

#include "mbed.h"
#include "ArduinoJson.h"

namespace mqtt {

class MQTTComponent {
public:
    MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic);
    virtual ~MQTTComponent();

    bool send_discovery();
    bool vpublish_state(const char* format, va_list args);
    bool publish_state(const char* format, ...);
    bool vpublish_state(const char* format, mbed::Callback<void(bool)> _cb, va_list args);
    bool publish_state(const char* format, mbed::Callback<void(bool)> _cb, ...);
    void set_timeout(int timeout) { m_timeout = timeout; }
    int get_timeout(int timeout) { return m_timeout; }
    void set_retain(bool retain) { m_retain = retain; }
    bool is_retrain() { return m_retain; }
    const char* get_friendly_name() { return m_friendly_name; }

protected:
    virtual const char* unique_id() = 0;
    virtual const char* component_type() = 0;
    virtual void set_discovery(JsonDocument& root) = 0;
    
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
    bool m_retain = true;
};

}

#endif
