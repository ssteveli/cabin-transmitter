#include "battery_monitor.h"
#include "mbed.h"
#include "log.h"
#include "local_config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"
#include "cloud_config.h"

mqtt::MQTTSensor battery_volts("cabin_battery", "hass:battery", "cabin/battery/volts/state");

Ticker bat_ticker;
bool bat_send = false;

AnalogIn bat_in(BAT);

void bat_read_data() {
    battery_volts.publish_state(bat_in.read_voltage());
}

void bat_flip_send_bit() {
    bat_send = true;
}

void bat_init() {
    bat_in.set_reference_voltage(3.304f);
    battery_volts.set_retain(true);
    battery_volts.set_unit_of_measurement("V");

    mqtt::mqtt_register_component(&battery_volts);

    bat_ticker.attach(callback(bat_flip_send_bit), cloud_config()->battery_interval);
}

void bat_loop() {
    if (bat_send) {
        bat_ticker.detach();
        bat_read_data();
        bat_send = false;
        bat_ticker.attach(callback(bat_flip_send_bit), cloud_config()->battery_interval);
    }
}