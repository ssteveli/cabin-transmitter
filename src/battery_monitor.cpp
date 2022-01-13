#include "battery_monitor.h"
#include "mbed.h"
#include "log.h"
#include "local_config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"
#include "cloud_config.h"

using duration = std::chrono::duration<int, std::milli>;

mqtt::MQTTSensor battery_volts("cabin_battery", "hass:battery", "cabin/battery/volts/state");

AnalogIn bat_in(BAT);
int bat_send_id = -1;
std::chrono::microseconds bat_interval;

void bat_publish_data() {
    battery_volts.publish_state(bat_in.read_voltage());
}

float bat_read_voltage() {
    return bat_in.read_voltage();
}

void bat_schedule_publish() {
    if (bat_send_id != -1) {
        mbed_event_queue()->cancel(bat_send_id);
        bat_send_id = -1;
    }

    bat_interval = cloud_config()->battery_interval;
    if (cloud_config()->battery_enabled) {
        std::chrono::duration<int, std::micro> d(cloud_config()->environment_interval);
        bat_send_id = mbed_event_queue()->call_every(std::chrono::duration_cast<duration>(d), bat_publish_data);
    }
}

void bat_init() {
    bat_in.set_reference_voltage(3.304f);
    battery_volts.set_retain(true);
    battery_volts.set_unit_of_measurement("V");

    mqtt::mqtt_register_component(&battery_volts);

    bat_schedule_publish();
}

void bat_loop() {
    bool currently_enabled = bat_send_id != -1;
    if (cloud_config()->battery_enabled != currently_enabled || cloud_config()->battery_interval != bat_interval) {
        bat_schedule_publish();
    }
}