#ifndef SRC_LOCAL_CONFIG_H_
#define SRC_LOCAL_CONFIG_H_

#ifdef TARGET_NUCLEO_L432KC
#define CB_I2C_SDA PB_7
#define CB_I2C_SCL PB_6
#define DHT22_OUT PA_5
#define LTE_PWR PA_7
#define LTE_RST PA_6
#define LTE_RX PA_10
#define LTE_TX PA_9
#define BAT PA_4
#define DL_CS PA_11
#define DL_MOSI PB_5
#define DL_MISO PB_4
#define DL_CLK PA_1
#endif

#ifdef TARGET_NUCLEO_L401RE
#define CB_I2C_SDA PB_9
#define CB_I2C_SCL PB_8
#define DHT22_OUT D4
#define LTE_PWR D5
#define LTE_RST D6
#define LTE_RX PA_10
#define LTE_TX PA_9
#define BAT PA_4
#endif

#define LTE_AUTO_DISCOVERY

typedef struct  {
    char mqtt_hostname[64];
} local_config_t;

void local_config_init();
const char* local_get_config_mqtt_hostname();

#endif