#include "log.h"
#include "mbed.h"

void log_init() {
}

void log_debug(const char *msg, ...) {
    if (LOG_LEVEL >= LOG_LEVEL_DEBUG) {
        va_list vl;
        va_start(vl, msg);
        vprintf(msg, vl);
        printf("\n");
        va_end(vl);
    }
}

void log_info(const char *msg, ...) {
    if (LOG_LEVEL >= LOG_LEVEL_INFO) {
        va_list vl;
        va_start(vl, msg);
        vprintf(msg, vl);
        printf("\n");
        va_end(vl);
    }
}