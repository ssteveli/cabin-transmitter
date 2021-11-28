#include "environment.h"
#include "lte.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"
#include "config.h"

using namespace std::chrono_literals;

DHT sensor(DHT22_OUT, DHT22);
EventQueue environment_queue(32 * EVENTS_EVENT_SIZE);
Thread environment_thread;

void environment_read_data() {
    int result = sensor.readData();

    if (result == 0) {
        lte_publish("cabin/env/temp", "%0.1f", NULL, TIMEOUT, sensor.ReadTemperature(FARENHEIT));
        lte_publish("cabin/env/humidity", "%0.1f", NULL, TIMEOUT, sensor.ReadHumidity());
    } else {
        log_debug("dhr error: %i", result);
    }
}

void environment_init() {
    environment_thread.start(callback(&environment_queue, &EventQueue::dispatch_forever));
    environment_thread.set_priority(osPriorityHigh);
    environment_queue.call_every(120s, environment_read_data);
}