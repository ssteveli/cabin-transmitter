#include "cloud_config.h"
#include <ArduinoJson.h>
#include "log.h"

cloud_config_t cloud_config_current;
 
void cloud_config_init() {
}

void cloud_config_set(std::string json) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        log_info("config deserialize failed: %s", error.c_str());
        return;
    }
    
    if (doc.containsKey("battery_enabled"))
      cloud_config_current.battery_enabled = doc["battery_enabled"].as<bool>();
    else
      cloud_config_current.battery_enabled = true;

    if (doc.containsKey("battery_interval")) {
      int duration = doc["battery_interval"].as<int>();
      auto ms = std::chrono::duration<int, std::milli>(duration);
      cloud_config_current.battery_interval = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    } else {
      cloud_config_current.battery_interval = DEFAULT_INTERVAL;
    }

    if (doc.containsKey("lte_enabled"))
      cloud_config_current.lte_enabled = doc["lte_enabled"].as<bool>();
    else
      cloud_config_current.lte_enabled = true;

    if (doc.containsKey("lte_interval")) {
      int duration = doc["lte_interval"].as<int>();
      auto ms = std::chrono::duration<int, std::milli>(duration);
      cloud_config_current.lte_interval = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    } else {
      cloud_config_current.lte_interval = DEFAULT_INTERVAL;
    }

    if (doc.containsKey("environment_enabled"))
      cloud_config_current.environment_enabled = doc["environment_enabled"].as<bool>();
    else
      cloud_config_current.environment_enabled = true;

    if (doc.containsKey("environment_interval")) {
      int duration = doc["environment_interval"].as<int>();
      auto ms = std::chrono::duration<int, std::milli>(duration);
      cloud_config_current.environment_interval = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    } else {
      cloud_config_current.environment_interval = DEFAULT_INTERVAL;
    }

    if (doc.containsKey("system_enabled"))
      cloud_config_current.system_enabled = doc["system_enabled"].as<bool>();
    else
      cloud_config_current.system_enabled = false;

    if (doc.containsKey("system_interval")) {
      int duration = doc["system_interval"].as<int>();
      auto ms = std::chrono::duration<int, std::milli>(duration);
      cloud_config_current.system_interval = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    } else {
      cloud_config_current.system_interval = DEFAULT_INTERVAL;
    }

    return;
}

cloud_config_t* cloud_config() {
  return &cloud_config_current;
}