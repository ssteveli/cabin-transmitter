#include "environment.h"
#include "mbed.h"
#include "log.h"
#include "sensors/DHT.h"
#include "local_config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"
#include "cloud_config.h"

using namespace std::chrono_literals;
using duration = std::chrono::duration<int, std::milli>;

DHT sensor(DHT22_OUT, DHT22);

mqtt::MQTTSensor temp("cabin_temperature", "hass:thermometer", "cabin/env/temp/state");
mqtt::MQTTSensor humidity("cabin_humidity", "hass:water-percent", "cabin/env/humidity/state");

std::chrono::microseconds env_interval;
int env_send_id = -1;
int env_last_read_result = 1;

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
}

void env_schedule_publish() {
    if (env_send_id != -1) {
        mbed_event_queue()->cancel(env_send_id);
    }

    env_interval = cloud_config()->environment_interval;
    std::chrono::duration<int, std::micro> d(cloud_config()->environment_interval);
    env_send_id = mbed_event_queue()->call_every(std::chrono::duration_cast<duration>(d), environment_publish_data);
}

void environment_init() {
    temp.set_retain(true);
    temp.set_unit_of_measurement("F");
    humidity.set_retain(true);
    humidity.set_unit_of_measurement("%");

    mqtt::mqtt_register_component(&temp);
    mqtt::mqtt_register_component(&humidity);

    env_read_data_from_sensor();
    mbed_event_queue()->call_every(120s, env_read_data_from_sensor);
    env_schedule_publish();
}

void environment_loop() {
    if (cloud_config()->environment_interval != env_interval) 
        env_schedule_publish();
}