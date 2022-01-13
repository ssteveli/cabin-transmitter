#include "data_logger.h"
#include "local_config.h"
#include "mbed.h"
#include "log.h"
#include "rtc.h"
#include "util/sd_card_helper.h"
#include "cloud_config.h"
#include "environment.h"
#include "battery_monitor.h"

#define FORCE_REFORMAT 0

BlockDevice *bd = BlockDevice::get_default_instance();
FileSystem *fs = FileSystem::get_default_instance();

Ticker data_logger_ticker;
bool data_record = false;

void data_logger_send_bit() {
    data_record = true;
}

void data_logger_init() {
    data_logger_ticker.attach(callback(data_logger_send_bit), cloud_config()->data_logger_interval);
}

void data_logger_log_data() {
    log_debug("data logger recording");
    FileSystem *fs = FileSystem::get_default_instance();

    char data_filename[32];
    sprintf(data_filename, "/%s/cabin-data.csv", fs->getName());
 
    FILE *f = fopen(data_filename, "a");
    if (f) {
        rtc_time_t t;
        if (rtc_read_time(&t) != 0) {
            log_debug("rtc read failed");
        }

        float value;
        if (environment_read_temperature(&value) == 0) {
            fprintf(f, "temp,%02d/%02d/%d %02d:%02d:%02d,%0.2f\n", t.month, t.date, t.year, t.hours, t.minute, t.second, value);
        } else {
            log_debug("failed to log temp data");
        }

        if (environment_read_humidity(&value) == 0) {
            fprintf(f, "humidity,%02d/%02d/%d %02d:%02d:%02d,%0.2f\n", t.month, t.date, t.year, t.hours, t.minute, t.second, value);
        } else {
            log_debug("failed to log humidity data");
        }

        fprintf(f, "bat,%02d/%02d/%d %02d:%02d:%02d,%0.2f\n", t.month, t.date, t.year, t.hours, t.minute, t.second, bat_read_voltage());
        fclose(f);
    } else {
        log_info("failed data logger, no data file");
        sd_card_helper_init(false, false);
    }

    fclose(f);
}

void data_logger_loop() {
     if (data_record) {
        data_logger_ticker.detach();
        data_record = false;
        data_logger_log_data();
        data_logger_ticker.attach(callback(data_logger_send_bit), cloud_config()->data_logger_interval);
    }
}