#ifndef SRC_CLOUD_CONFIG_H_
#define SRC_CLOUD_CONFIG_H_

#include "mbed.h"
#include <string>

#define DEFAULT_INTERVAL 120s

typedef struct {
    bool environment_enabled = true;
    std::chrono::microseconds environment_interval = DEFAULT_INTERVAL;
    bool system_enabled = false;
    std::chrono::microseconds system_interval = DEFAULT_INTERVAL;
    bool battery_enabled = true;
    std::chrono::microseconds battery_interval = DEFAULT_INTERVAL;
    bool lte_enabled = true;
    std::chrono::microseconds lte_interval = DEFAULT_INTERVAL;
    bool data_logger_enabled = true;
    std::chrono::microseconds data_logger_interval = 5s;
} cloud_config_t;

void cloud_config_init();
void cloud_config_set(std::string json);
cloud_config_t* cloud_config();

#endif