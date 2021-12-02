#ifndef SRC_MQTT_COMPONENT_
#define SRC_MQTT_COMPONENT_

namespace mqtt {

class MQTTComponent {
public:
    MQTTComponent(const char* friendly_name, const char* icon, const char* state_topic);
    virtual ~MQTTComponent();

    virtual bool send_discovery() = 0;
    bool publish_state(const char* format, ...);

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
};

}

#endif
