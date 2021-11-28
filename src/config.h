#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#define LTE_RX PA_10
#define LTE_TX PA_9
#define LTE_PWR D5
#define LTE_RST D6
#define DHT22_OUT D4
#define CB_I2C_SDA PB_9
#define CB_I2C_SCL PB_8

void config_init();
const char* get_config_mqtt_hostname();

#endif