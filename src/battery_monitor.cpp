#include "battery_monitor.h"
#include "mbed.h"
#include "log.h"
#include "config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"

mqtt::MQTTSensor battery("cabin_battery", "hass:battery", "cabin/env/battery/state");

#define BAT_POLLING_PERIOD 120s
Ticker bat_ticker;
bool bat_send = false;

void bat_read_data() {
    float volts = 0.0f;
    battery.publish_state(volts);
}

void bat_flip_send_bit() {
    bat_send = true;
}

void bat_init() {
    battery.set_retain(true);
    battery.set_unit_of_measurement("V");

    mqtt::mqtt_register_component(&battery);

    bat_ticker.attach(callback(bat_flip_send_bit), BAT_POLLING_PERIOD);
}

void bat_loop() {
    if (bat_send) {
        bat_ticker.detach();
        bat_read_data();
        bat_send = false;
        bat_ticker.attach(callback(bat_flip_send_bit), BAT_POLLING_PERIOD);
    }
}