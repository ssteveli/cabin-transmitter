#include "system.h"
#include "mbed.h"
#include "lte.h"
#include <inttypes.h>
#include "events.h"

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

void os_reporting_worker() {
    while (true) {
        event_flags.wait_any(FLAG_SYSTEM_READY);
        
        mbed_stats_sys_t sys_stats;
        mbed_stats_sys_get(&sys_stats);

        lte_publish("cabin/system/os/version",  "%" PRId32 "", NULL, TIMEOUT,  sys_stats.os_version);
    }
}

void system_read_data() {
    // heap
    mbed_stats_heap_get(&heap_info);

    lte_publish("cabin/system/heap/current_size", "%lb", NULL, TIMEOUT, heap_info.current_size);
    lte_publish("cabin/system/heap/max_size", "%lb", NULL, TIMEOUT, heap_info.max_size);
    lte_publish("cabin/system/heap/total_size", "%lb", NULL, TIMEOUT, heap_info.total_size);

    // CPU
    mbed_stats_cpu_get(&stats);
    uint64_t diff_usec = (stats.idle_time - prev_idle_time);
    uint8_t idle = (diff_usec * 100) / (SAMPLE_TIME_MS*1000);
    uint8_t usage = 100 - ((diff_usec * 100) / (SAMPLE_TIME_MS*1000));
    prev_idle_time = stats.idle_time;

    lte_publish("cabin/system/cpu/uptime", "%llb", NULL, TIMEOUT, stats.uptime);
    lte_publish("cabin/system/cpu/idle_time", "%llb", NULL, TIMEOUT, stats.idle_time);
    lte_publish("cabin/system/cpu/sleep_time", "%llb", NULL, TIMEOUT, stats.sleep_time);
    lte_publish("cabin/system/cpu/idle", "%d%%", NULL, TIMEOUT, idle);
    lte_publish("cabin/system/cpu/up", "%d%%", NULL, TIMEOUT, usage);
}

void system_init() {
    system_thread.start(callback(&system_queue, &EventQueue::dispatch_forever));
    system_queue.call_every(60s, system_read_data);
    system_os_reporting_thread.start(callback(os_reporting_worker));
}