#include "config.h"
#include "mbed.h"
#include "log.h"
#include "KVStore.h"
#include "kvstore_global_api.h"
#include "DeviceKey.h"

#define KV_VALUE_LENGTH 64
#define KV_KEY_LENGTH 32

#define err_code(res) MBED_GET_ERROR_CODE(res)
#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

const char *config_kv_enabled = "/kv/enabled";
const char *config_kv_mqtt_hostname = "/kv/mqtt-hostname";
static char config_mqtt_hostname[128] = {0};

const char* get_config_mqttt_hostname() {
    return config_mqtt_hostname;
}

void rebuild_kv_store() {    
    int res = MBED_ERROR_NOT_READY;

    log_debug("kv_reset");
    res = kv_reset("/kv/");
    log_debug("kv_reset -> %d", err_code(res));

    const char *enabled = "enabled";
    res = kv_set(config_kv_enabled, enabled, strlen(enabled), 0);
    log_debug("kv_set -> %d\n", err_code(res));

    char mqtt_hostname[128] = {0};
    printf("Enter MQTT Hostname: ");
    scanf("%s", mqtt_hostname);
    if (strlen(mqtt_hostname) > 0) {
        printf("Storing ... ");
        res = kv_set(config_kv_mqtt_hostname, mqtt_hostname, strlen(mqtt_hostname), 0);
        log_debug("config_kv_mqtt_hostname -> %d\n", err_code(res));
    }
}

void config_init() {
    kv_info_t info;
    int res = MBED_ERROR_NOT_READY;
    res = kv_get_info(config_kv_enabled, &info);

    if (res == MBED_ERROR_ITEM_NOT_FOUND) {
        log_debug("config kv store is not ready, building");
        rebuild_kv_store();        
    }

    size_t actual_size;
    res = kv_get(config_kv_mqtt_hostname, config_mqtt_hostname, sizeof(config_mqtt_hostname), &actual_size);
    printf("config_mqtt_hostname value: %s\n", config_mqtt_hostname);

    log_debug("config init completed");
}