#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#ifdef TARGET_NUCLEO_L432KC
#define CB_I2C_SDA PB_4
#define CB_I2C_SCL PA_7
#define DHT22_OUT PB_1
#define LTE_PWR PA_12
#define LTE_RST PB_0
#define LTE_RX PB_7
#define LTE_TX PB_6
#endif

#ifdef TARGET_NUCLEO_L401RE
#define CB_I2C_SDA PB_9
#define CB_I2C_SCL PB_8
#define DHT22_OUT D4
#define LTE_PWR D5
#define LTE_RST D6
#define LTE_RX PA_10
#define LTE_TX PA_9
#endif

#define BE_LIKE_ESPHOME
#define LTE_AUTO_DISCOVERY

void config_init();
const char* get_config_mqtt_hostname();

#endif