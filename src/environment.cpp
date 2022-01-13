#include "environment.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"
#include "local_config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"
#include "cloud_config.h"

using namespace std::chrono_literals;

DHT sensor(DHT22_OUT, DHT22);

mqtt::MQTTSensor temp("cabin_temperature", "hass:thermometer", "cabin/env/temp/state");
mqtt::MQTTSensor humidity("cabin_humidity", "hass:water-percent", "cabin/env/humidity/state");

Ticker env_ticker;
bool env_send = false;

Ticker env_reader;
int env_last_read_result = 99;
bool env_need_read = true;

void environment_publish_data() {
    if (env_last_read_result == 0) {
        temp.publish_state(sensor.ReadTemperature(FARENHEIT));
        humidity.publish_state(sensor.ReadHumidity());
    } else {
        log_debug("dht error: %i", env_last_read_result);
    }
}

int environment_read_temperature(float *f) {
    if (env_last_read_result == 0)
        *f = sensor.ReadTemperature(FARENHEIT);
    
    return env_last_read_result;
}

int environment_read_humidity(float *f) {
    if (env_last_read_result == 0) 
        *f = sensor.ReadHumidity();

    return env_last_read_result;
}

void env_flip_send_bit() {
    env_send = true;
}

void env_flip_need_bit() {
    env_need_read = true;
}

void env_read_data_from_sensor() {
    int result = -1;
    int attempts = 0;

    while (result != 0 && ++attempts < 10) {
        result = sensor.readData();

        if (result == 0) {
            break;
        }

        ThisThread::sleep_for(1s);
    }

    log_debug("dht22 read result: %d", result);
    env_last_read_result = result;
    env_need_read = false;
}

void environment_init() {
    temp.set_retain(true);
    temp.set_unit_of_measurement("F");
    humidity.set_retain(true);
    humidity.set_unit_of_measurement("%");

    mqtt::mqtt_register_component(&temp);
    mqtt::mqtt_register_component(&humidity);

    env_ticker.attach(callback(env_flip_send_bit), cloud_config()->environment_interval);
    env_reader.attach(callback(env_flip_need_bit), 60s);
}

void environment_loop() {
    if (env_need_read) {
        env_read_data_from_sensor();
    }

    if (env_send) {
        env_ticker.detach();
        environment_publish_data();
        env_send = false;
        env_ticker.attach(callback(env_flip_send_bit), cloud_config()->environment_interval);
    }
}