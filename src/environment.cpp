#include "environment.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"
#include "config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"

using namespace std::chrono_literals;

DHT sensor(DHT22_OUT, DHT22);

mqtt::MQTTSensor temp("cabin_temperature", "hass:thermometer", "cabin/env/temp/state");
mqtt::MQTTSensor humidity("cabin_humidity", "hass:water-percent", "cabin/env/humidity/state");

#define ENV_POLLING_PERIOD 320s
Ticker env_ticker;
bool env_send = false;

void environment_read_data() {
    int result = -1;
    int attempts = 0;

    while (result != 0 && ++attempts < 10) {
        result = sensor.readData();

        if (result == 0) {
            break;
        }
    }

    if (result == 0) {
        temp.publish_state(sensor.ReadTemperature(FARENHEIT));
        humidity.publish_state(sensor.ReadHumidity());
    } else {
        log_debug("dhr error: %i", result);
    }
}

void env_flip_send_bit() {
    env_send = true;
}

void environment_init() {
    temp.set_retain(true);
    temp.set_unit_of_measurement("F");
    humidity.set_retain(true);
    humidity.set_unit_of_measurement("%");

    mqtt::mqtt_register_component(&temp);
    mqtt::mqtt_register_component(&humidity);

    env_ticker.attach(callback(env_flip_send_bit), ENV_POLLING_PERIOD);
}

void environment_loop() {
    if (env_send) {
        env_ticker.detach();
        environment_read_data();
        env_send = false;
        env_ticker.attach(callback(env_flip_send_bit), ENV_POLLING_PERIOD);
    }
}