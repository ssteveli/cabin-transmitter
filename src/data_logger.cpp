#include "data_logger.h"
#include "local_config.h"
#include "mbed.h"
#include "log.h"
#include "rtc.h"
#include "util/sd_card_helper.h"
#include "cloud_config.h"
#include "environment.h"
#include "battery_monitor.h"

using duration = std::chrono::duration<int, std::milli>;

BlockDevice *bd = BlockDevice::get_default_instance();
FileSystem *fs = FileSystem::get_default_instance();

std::chrono::microseconds data_interval;
int data_send_id = -1;

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

void data_schedule_publish() {
    if (data_send_id != -1) {
        mbed_event_queue()->cancel(data_send_id);
        data_send_id = -1;
    }

    data_interval = cloud_config()->data_logger_interval;
    if (cloud_config()->data_logger_enabled) {
        std::chrono::duration<int, std::micro> d(cloud_config()->data_logger_interval);
        data_send_id = mbed_event_queue()->call_every(std::chrono::duration_cast<duration>(d), data_logger_log_data);
    }
}

void data_logger_init() {
    data_schedule_publish();
}

void data_logger_loop() {
    bool currently_enabled = data_send_id != -1;
    if (cloud_config()->data_logger_enabled != currently_enabled || cloud_config()->data_logger_interval != data_interval) {
        data_schedule_publish();
    }
}