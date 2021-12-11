#include "battery_monitor.h"
#include "mbed.h"
#include "log.h"
#include "config.h"
#include "mqtt/mqtt_sensor.h"
#include "mqtt/mqtt_component_discovery.h"

mqtt::MQTTSensor battery_volts("cabin_battery", "hass:battery", "cabin/battery/volts/state");

#define BAT_POLLING_PERIOD 120s
Ticker bat_ticker;
bool bat_send = false;

void bat_read_data() {
    float volts = 25.7f;
    battery_volts.publish_state(volts);
}

void bat_flip_send_bit() {
    bat_send = true;
}

void bat_init() {
    battery_volts.set_retain(true);
    battery_volts.set_unit_of_measurement("V");

    mqtt::mqtt_register_component(&battery_volts);

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