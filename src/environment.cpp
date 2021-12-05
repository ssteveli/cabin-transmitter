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

#ifdef BE_LIKE_ESPHOME
mqtt::MQTTSensor temp("cabin_temperature", "hass:thermometer", "cabin/env/temp/state");
mqtt::MQTTSensor humidity("cabin_humidity", "hass:water-percent", "cabin/env/humidity/state");
#endif

#define ENV_POLLING_PERIOD 5s
Ticker env_ticker;
bool env_send = false;

void environment_read_data() {
    int result = sensor.readData();

    if (result == 0) {
        #ifdef BE_LIKE_ESPHOME
        temp.publish_state("%0.1f", sensor.ReadTemperature(FARENHEIT));
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

void env_flip_send_bit() {
    env_send = true;
}

void environment_init() {
    #ifdef BE_LIKE_ESPHOME
    temp.set_retain(true);
    temp.set_unit_of_measurement("F");
    humidity.set_retain(true);
    humidity.set_unit_of_measurement("%");

    mqtt::mqtt_register_component(&temp);
    mqtt::mqtt_register_component(&humidity);
    #endif

    env_ticker.attach(callback(env_flip_send_bit), ENV_POLLING_PERIOD);
}

void environment_loop() {
    if (env_send) {
        environment_read_data();
        env_send = false;
    }
}