#ifndef SRC_LTE_H_
#define SRC_LTE_H_

#include "mbed.h"

#define MAX_OPERATORS 5
#define TIMEOUT 1000

#define NUM_SUPPORTED_BAUD 6
const unsigned int LTE_SHIELD_SUPPORTED_BAUD[NUM_SUPPORTED_BAUD] = {
        115200,
        9600,
        19200,
        38400,
        57600,
        230400
};

typedef enum {
        INIT,
        READY,
        BAUD_SELECTION,
        OPERATOR_REGISTRATION, 
        MQTT_LOGIN
} lte_state_t;

typedef enum {
    MNO_INVALID = -1,
    MNO_SW_DEFAULT = 0,
    MNO_SIM_ICCD = 1,
    MNO_ATT = 2,
    MNO_VERIZON = 3,
    MNO_TELSTRA = 4,
    MNO_TMO = 5,
    MNO_CT = 6
} mobile_network_operator_t;

constexpr mobile_network_operator_t MOBILE_NETWORK_OPERATOR = MNO_SW_DEFAULT;
constexpr char APN[] = "hologram";

constexpr char LTE_SHIELD_COMMAND_BAUD[] = "AT+IPR";
constexpr char LTE_SHIELD_OPERATOR_SELECTION[] = "AT+COPS";

void lte_discover_baud_rate();
void lte_operator_registration();
void lte_mqtt_login();
void lte_init();
bool lte_publish(const char *topic, const char *value, mbed::Callback<void(bool)> _cb, int timeout = TIMEOUT, ...);
bool lte_send(const char *command, const char *expected_result, mbed::Callback<void(bool)> _cb = NULL, int timeout = TIMEOUT);

#endif