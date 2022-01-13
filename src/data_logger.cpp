#include "data_logger.h"
#include "local_config.h"
#include "mbed.h"
#include "log.h"
#include "rtc.h"

#define FORCE_REFORMAT 0

BlockDevice *bd = BlockDevice::get_default_instance();
FileSystem *fs = FileSystem::get_default_instance();

void data_logger_init() {
 
}

void data_logger_loop() {

}