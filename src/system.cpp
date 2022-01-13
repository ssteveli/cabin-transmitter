#include "system.h"
#include "mbed.h"
#include <inttypes.h>
#include "events.h"
#include "log.h"
#include "mqtt/mqtt_sensor.h"
#include "config.h"
#include "mqtt/mqtt_component_discovery.h"
#include "cloud_config.h"

using duration = std::chrono::duration<int, std::milli>;

mbed_stats_heap_t heap_info;
mbed_stats_cpu_t stats;
uint64_t prev_idle_time = 0;

#if !defined(MBED_SYS_STATS_ENABLED)
#error [NOT_SUPPORTED] System statistics not supported
#endif

mqtt::MQTTSensor os_version("cabin_os_version", "hass:account", "cabin/system/os-version/state");

mqtt::MQTTSensor heap_current_size("cabin_heap_current_size", "hass:account", "cabin/system/heap/current_size/state");
mqtt::MQTTSensor heap_max_size("cabin_heap_max_size", "hass:account", "cabin/system/heap/max_size/state");
mqtt::MQTTSensor heap_total_size("cabin_heap_total_size", "hass:account", "cabin/system/heap/total_size/state");

int sys_send_id = -1;
std::chrono::microseconds sys_interval;

void system_startup() {
    mbed_stats_sys_t sys_stats;
    mbed_stats_sys_get(&sys_stats);

    os_version.publish_state(sys_stats.os_version);
}

void sys_publish_data() {
    // heap
    mbed_stats_heap_get(&heap_info);

    heap_current_size.publish_state(heap_info.current_size);
    heap_max_size.publish_state(heap_info.max_size);
    heap_total_size.publish_state(heap_info.total_size);
}

void sys_schedule_publish() {
    if (sys_send_id != -1) {
        mbed_event_queue()->cancel(sys_send_id);
        sys_send_id = -1;
    }

    sys_interval = cloud_config()->system_interval;
    if (cloud_config()->system_enabled) {
        std::chrono::duration<int, std::micro> d(cloud_config()->environment_interval);
        sys_send_id = mbed_event_queue()->call_every(std::chrono::duration_cast<duration>(d), sys_publish_data);
    }
}

void system_init() {
    mqtt::mqtt_register_component(&os_version);
    mqtt::mqtt_register_component(&heap_current_size);
    mqtt::mqtt_register_component(&heap_max_size);
    mqtt::mqtt_register_component(&heap_total_size);

    sys_schedule_publish();
}

void system_loop() {
    bool currently_enabled = sys_send_id != -1;
    if (cloud_config()->system_enabled != currently_enabled || cloud_config()->system_interval != sys_interval) {
        sys_schedule_publish();
    }
}