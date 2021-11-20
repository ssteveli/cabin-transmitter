#include "environment.h"
#include "lte.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"

using namespace std::chrono_literals;

DHT sensor(D4, DHT22);
EventQueue environment_queue(32 * EVENTS_EVENT_SIZE);
Thread environment_thread;

void environment_read_data() {
    int result = sensor.readData();

    if (result == 0) {
        log_debug("successfully read environment data");

        printf("humidity: %.2f, temp: %.2f\n", sensor.ReadHumidity(), sensor.ReadTemperature(FARENHEIT));

        char temp_s[32] = {0};
        sprintf(temp_s, "%0.2f", sensor.ReadTemperature(FARENHEIT));
        lte_publish("cabin/temp", temp_s, NULL);

        char humidity_s[32] = {0};
        sprintf(humidity_s, "%0.2f", sensor.ReadHumidity());
        lte_publish("cabin/humidity", temp_s, NULL);

    } else {
        log_debug("dhr error: %i", result);
    }
}

void environment_init() {
    environment_thread.start(callback(&environment_queue, &EventQueue::dispatch_forever));
    environment_thread.set_priority(osPriorityHigh);
    environment_queue.call_every(10s, environment_read_data);
}