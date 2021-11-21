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

        char *temp_s = (char *)calloc(32, sizeof(char));
        sprintf(temp_s, "%0.2f", sensor.ReadTemperature(FARENHEIT));
        if (lte_publish("cabin/temp", temp_s, NULL)) {
            log_debug("published temp on %p", ThisThread::get_id());
        } else {
            log_debug("published temp failed on %p", ThisThread::get_id());
        }

        char *humidity_s = (char *)calloc(32, sizeof(char));
        sprintf(humidity_s, "%0.2f", sensor.ReadHumidity());
        if (lte_publish("cabin/humidity", humidity_s, NULL)) {
            log_debug("published humidity on %p", ThisThread::get_id());
        } else {
            log_debug("published humidity failed on %p", ThisThread::get_id());
        }

        free(temp_s);
        free(humidity_s);
    } else {
        log_debug("dhr error: %i", result);
    }
}

void environment_init() {
    environment_thread.start(callback(&environment_queue, &EventQueue::dispatch_forever));
    environment_thread.set_priority(osPriorityHigh);
    environment_queue.call_every(10s, environment_read_data);
}