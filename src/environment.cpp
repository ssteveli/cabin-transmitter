#include "environment.h"
#include "lte.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"
#include "config.h"
#include "rtc.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"

using namespace std::chrono_literals;

DHT sensor(DHT22_OUT, DHT22);
EventQueue environment_queue(32 * EVENTS_EVENT_SIZE);
Thread environment_thread;

#ifdef BE_LIKE_ESPHOME
mqtt::MQTTSensor temp("cabin_temperature", "hass:thermometer", "cabin/env/temp/state");
mqtt::MQTTSensor humidity("cabin_humidity", "hass:water-percent", "cabin/env/humidity/state");
#endif

void environment_read_data() {
    int result = sensor.readData();

    if (result == 0) {
        #ifdef BE_LIKE_ESPHOME
        temp.publish_state("%0.1f", sensor.ReadTemperature(CELCIUS));
        humidity.publish_state("%0.1f", sensor.ReadHumidity());
        #else
        time_t t = rtc_read_time();
        lte_publish("cabin/env/temp", "%ld,%0.1f", NULL, TIMEOUT, t, sensor.ReadTemperature(FARENHEIT));
        lte_publish("cabin/env/humidity", "%ld,%0.1f", NULL, TIMEOUT, t, sensor.ReadHumidity());
        #endif
    } else {
        log_debug("dhr error: %i", result);
    }
}

void environment_init() {
    #ifdef BE_LIKE_ESPHOME
    temp.set_retain(true);
    temp.set_unit_of_measurement("°C");
    humidity.set_retain(true);
    humidity.set_unit_of_measurement("%");

    mqtt::mqtt_register_component(&temp);
    mqtt::mqtt_register_component(&humidity);
    #endif
    
    environment_thread.start(callback(&environment_queue, &EventQueue::dispatch_forever));
    environment_thread.set_priority(osPriorityHigh);
    environment_queue.call_every(120s, environment_read_data);
}