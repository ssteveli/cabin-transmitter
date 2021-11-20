#ifndef SRC_LOG_H_
#define SRC_LOG_H_

#include <stdint.h>

const uint8_t LOG_LEVEL_INFO = 1;
const uint8_t LOG_LEVEL_DEBUG = 2;
const uint8_t LOG_LEVEL = 2;

void log_init();
void log_debug(const char *msg, ...);
void log_info(const char *msg, ...);

#endif