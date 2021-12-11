#include "system.h"
#include "mbed.h"
#include <inttypes.h>
#include "events.h"
#include "log.h"
#include "mqtt/mqtt_sensor.h"
#include "config.h"
#include "mqtt/mqtt_component_discovery.h"

#define SAMPLE_TIME_MS   2000

mbed_stats_heap_t heap_info;
mbed_stats_cpu_t stats;
uint64_t prev_idle_time = 0;

#if !defined(MBED_SYS_STATS_ENABLED)
#error [NOT_SUPPORTED] System statistics not supported
#endif

#ifdef BE_LIKE_ESPHOME
mqtt::MQTTSensor os_version("cabin_os_version", "hass:account", "cabin/system/os-version/state");

mqtt::MQTTSensor heap_current_size("cabin_heap_current_size", "hass:account", "cabin/system/heap/current_size/state");
mqtt::MQTTSensor heap_max_size("cabin_heap_max_size", "hass:account", "cabin/system/heap/max_size/state");
mqtt::MQTTSensor heap_total_size("cabin_heap_total_size", "hass:account", "cabin/system/heap/total_size/state");

mqtt::MQTTSensor cpu_uptime("cabin_cpu_uptime_t", "hass:account", "cabin/system/cpu/uptime/state");
mqtt::MQTTSensor cpu_idle_time("cabin_cpu idle_t", "hass:account", "cabin/system/cpu/idle_time/state");
mqtt::MQTTSensor cpu_sleep_time("cabin_cpu_sleep_t", "hass:account", "cabin/system/cpu/sleep_time/state");
mqtt::MQTTSensor cpu_idle("cabin_cpu_idle_p", "hass:account", "cabin/system/cpu/idle_percentage/state");
mqtt::MQTTSensor cpu_up("cabin_cpu_uptime_p", "hass:account", "cabin/system/cpu/uptime_percentage/state");
#endif

#define SYS_POLLING_PERIOD 320s
Ticker sys_ticker;
bool sys_send = false;

void system_startup() {
    mbed_stats_sys_t sys_stats;
    mbed_stats_sys_get(&sys_stats);

    os_version.publish_state(sys_stats.os_version);
}

void system_read_data() {
    // heap
    mbed_stats_heap_get(&heap_info);

    heap_current_size.publish_state(heap_info.current_size);
    heap_max_size.publish_state(heap_info.max_size);
    heap_total_size.publish_state(heap_info.total_size);

    // CPU
    mbed_stats_cpu_get(&stats);
    uint64_t diff_usec = (stats.idle_time - prev_idle_time);
    uint8_t idle = (diff_usec * 100) / (SAMPLE_TIME_MS*1000);
    uint8_t usage = 100 - ((diff_usec * 100) / (SAMPLE_TIME_MS*1000));
    prev_idle_time = stats.idle_time;

    cpu_uptime.publish_state(stats.uptime);
    cpu_idle_time.publish_state(stats.idle_time);
    cpu_sleep_time.publish_state(stats.sleep_time);
    cpu_idle.publish_state(idle);
    cpu_up.publish_state(usage);
}

void system_flip_send_bit() {
    sys_send = true;
}

void system_init() {
    mqtt::mqtt_register_component(&os_version);
    mqtt::mqtt_register_component(&heap_current_size);
    mqtt::mqtt_register_component(&heap_max_size);
    mqtt::mqtt_register_component(&heap_total_size);
    mqtt::mqtt_register_component(&cpu_uptime);
    mqtt::mqtt_register_component(&cpu_idle_time);
    mqtt::mqtt_register_component(&cpu_sleep_time);
    mqtt::mqtt_register_component(&cpu_idle);
    mqtt::mqtt_register_component(&cpu_up);
    
    sys_ticker.attach(&system_flip_send_bit, SYS_POLLING_PERIOD);
}

void system_loop() {
    if (sys_send) {
        sys_ticker.detach();
        system_read_data();
        sys_send = false;
        sys_ticker.attach(&system_flip_send_bit, SYS_POLLING_PERIOD);
    }
}