#include "system.h"
#include "mbed.h"
#include "lte.h"
#include <inttypes.h>
#include "events.h"
#include "log.h"
#include "rtc.h"
#include "mqtt/mqtt_sensor.h"
#include "config.h"

#define MAX_THREAD_INFO 10
#define SAMPLE_TIME_MS   2000

EventQueue system_queue(32 * EVENTS_EVENT_SIZE);
Thread system_thread;
Thread system_os_reporting_thread;

mbed_stats_heap_t heap_info;
mbed_stats_stack_t stack_info[ MAX_THREAD_INFO ];
mbed_stats_cpu_t stats;
uint64_t prev_idle_time = 0;

#if !defined(MBED_SYS_STATS_ENABLED)
#error [NOT_SUPPORTED] System statistics not supported
#endif

#ifdef BE_LIKE_ESPHOME
mqtt::MQTTSensor os_version("cabin os version", "hass:account", "cabin/system/os-version/state");

mqtt::MQTTSensor heap_current_size("cabin heap current size", "hass:account", "cabin/system/heap/current_size/state");
mqtt::MQTTSensor heap_max_size("cabin heap max size", "hass:account", "cabin/system/heap/max_size/state");
mqtt::MQTTSensor heap_total_size("cabin heap total size", "hass:account", "cabin/system/heap/total_size/state");

mqtt::MQTTSensor cpu_uptime("cabin cpu uptime", "hass:account", "cabin/system/cpu/uptime/state");
mqtt::MQTTSensor cpu_idle_time("cabin cpu uptime", "hass:account", "cabin/system/cpu/idle_time/state");
mqtt::MQTTSensor cpu_sleep_time("cabin cpu uptime", "hass:account", "cabin/system/cpu/sleep_time/state");
mqtt::MQTTSensor cpu_idle("cabin cpu uptime", "hass:account", "cabin/system/cpu/idle_percentage/state");
mqtt::MQTTSensor cpu_up("cabin cpu uptime", "hass:account", "cabin/system/cpu/uptime_percentage/state");
#endif

void os_reporting_worker() {
    while (true) {
        log_debug("os stats waiting for ready flag");
        event_flags.wait_any(FLAG_SYSTEM_READY);
        log_debug("reporting os information");

        mbed_stats_sys_t sys_stats;
        mbed_stats_sys_get(&sys_stats);

        #ifdef BE_LIKE_ESPHOME
        os_version.publish_state("%" PRId32 "", sys_stats.os_version);
        #else
        lte_publish("cabin/system/os/version",  "%" PRId32 "", NULL, TIMEOUT,  sys_stats.os_version);
        #endif
    }
}

void system_read_data() {
    // heap
    mbed_stats_heap_get(&heap_info);

    #ifdef BE_LIKE_ESPHOME
    heap_current_size.publish_state("%ld", heap_info.current_size);
    heap_max_size.publish_state("%ld", heap_info.max_size);
    heap_total_size.publish_state("%ld", heap_info.total_size);
    #else
    time_t t = rtc_read_time();

    lte_publish("cabin/system/heap/current_size", "%ld,%ld", NULL, TIMEOUT, t, heap_info.current_size);
    lte_publish("cabin/system/heap/max_size", "%ld,%ld", NULL, TIMEOUT, t, heap_info.max_size);
    lte_publish("cabin/system/heap/total_size", "%ld,%ld", NULL, TIMEOUT, t, heap_info.total_size);
    #endif

    // CPU
    mbed_stats_cpu_get(&stats);
    uint64_t diff_usec = (stats.idle_time - prev_idle_time);
    uint8_t idle = (diff_usec * 100) / (SAMPLE_TIME_MS*1000);
    uint8_t usage = 100 - ((diff_usec * 100) / (SAMPLE_TIME_MS*1000));
    prev_idle_time = stats.idle_time;

    #ifdef BE_LIKE_ESPHOME
    cpu_uptime.publish_state("%ld", stats.uptime);
    cpu_idle_time.publish_state("%ld", stats.idle_time);
    cpu_sleep_time.publish_state("%ld", stats.sleep_time);
    cpu_idle.publish_state("%ld", idle);
    cpu_up.publish_state("%ld", usage);
    #else
    time_t t = rtc_read_time();

    lte_publish("cabin/system/cpu/uptime", "%ld,%ld", NULL, TIMEOUT, t, stats.uptime);
    lte_publish("cabin/system/cpu/idle_time", "%ld,%ld", NULL, TIMEOUT, t, stats.idle_time);
    lte_publish("cabin/system/cpu/sleep_time", "%ld,%ld", NULL, TIMEOUT, t, stats.sleep_time);
    lte_publish("cabin/system/cpu/idle", "%ld,%d%%", NULL, TIMEOUT, t, idle);
    lte_publish("cabin/system/cpu/up", "%ld,%d%%", NULL, TIMEOUT, t, usage);
    #endif
}

void system_init() {
    system_thread.start(callback(&system_queue, &EventQueue::dispatch_forever));
    system_queue.call_every(320s, system_read_data);
    system_os_reporting_thread.start(os_reporting_worker);
}