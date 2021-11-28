#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#define LTE_RX PA_10
#define LTE_TX PA_9
#define LTE_PWR D5
#define LTE_RST D6
#define DHT22_OUT D4

void config_init();
const char* get_config_mqtt_hostname();

#endif