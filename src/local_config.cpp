#include "local_config.h"
#include "mbed.h"
#include "log.h"
#include "ArduinoJson.h"
#include "util/sd_card_helper.h"

local_config_t config;

const char* local_get_config_mqtt_hostname() {
    return config.mqtt_hostname;
}

void local_config_save() {
    FileSystem *fs = FileSystem::get_default_instance();

    char config_filename[32];
    sprintf(config_filename, "/%s/config.json", fs->getName());
    log_debug("config filename on save: %s", config_filename);

    log_debug("saving config file %s", config_filename);
    FILE *f = fopen(config_filename, "w+");

    StaticJsonDocument<256> doc;
    doc["mqtt_hostname"] = config.mqtt_hostname;
    
    std::string json;
    serializeJsonPretty(doc, json);
    int bytes_written = fprintf(f, json.c_str());
    log_debug("config write byte count: %d", bytes_written);

    fclose(f);

    if (bytes_written == 0) {
        log_info("no bytes written creating %s, unable to continue", config_filename);
        while (1) {}
    }
}

void local_config_load() {
    FileSystem *fs = FileSystem::get_default_instance();

    char config_filename[32];
    sprintf(config_filename, "/%s/config.json", fs->getName());
    log_debug("config filename on load: %s", config_filename);

    FILE *f = fopen(config_filename, "r+");
    if (f) {
        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        std::string json;
        json.resize(size);

        rewind(f);
        fread(&json[0], 1, size, f);
        fclose(f);
        log_debug("config loaded: \n\n%s\n", json.c_str());

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, json);

        fclose(f);
        if (error) {
            log_info("failed to read %s, using default configuration", config_filename);
        }

        strcpy(config.mqtt_hostname, doc["mqtt_hostname"] | "");
    } else {
        log_debug("no configuration file was found!!");
    }
}

void local_config_read_required_data() {  
    log_info("MISSING REQUIRED CONFIGURATION DATA \n\n");

    char mqtt_hostname[sizeof(config.mqtt_hostname)] = {0};
    printf("Enter MQTT Hostname: ");
    scanf("%s", mqtt_hostname);
    if (strlen(mqtt_hostname) > 0) {
        strcpy(config.mqtt_hostname, mqtt_hostname);
        local_config_save();
    }
}

void local_config_init() {
    FileSystem *fs = FileSystem::get_default_instance();

    char config_filename[32];
    sprintf(config_filename, "/%s/config.json", fs->getName());
    log_debug("config filename: %s", config_filename);

    FILE *f = fopen(config_filename, "r+");

    if (!f) {
        // create a new configuration file
        strcpy(config.mqtt_hostname, "");
        local_config_save();
    } else {
        fclose(f);
        local_config_load();
    }

    // allow the user to enter the required data if necessary
    if (strlen(config.mqtt_hostname) == 0) {
        local_config_read_required_data();
    }

    if (strlen(config.mqtt_hostname) == 0) {
        log_info("no mqtt_hostname set, unable to configure");
        while (1) {}
    }

    printf("config_mqtt_hostname value: %s\n", config.mqtt_hostname);

    log_debug("config init completed");
}
