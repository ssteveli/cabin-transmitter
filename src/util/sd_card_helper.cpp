#include "sd_card_helper.h"
#include "mbed.h"
#include "log.h"
#include "FATFileSystem.h"
#include "BlockDevice.h"
#include "SDBlockDevice.h"
#include "local_config.h"

BlockDevice *BlockDevice::get_default_instance() {
    static SDBlockDevice default_bd(DL_MOSI, DL_MISO, DL_CLK, DL_CS);

    return &default_bd;
}

FileSystem *FileSystem::get_default_instance() {
    static FATFileSystem default_fs("sd", BlockDevice::get_default_instance());

    return &default_fs;
}

int sd_card_helper_init(bool halt_on_failure, bool allow_reformat) {
    BlockDevice *bd = BlockDevice::get_default_instance();
    FileSystem *fs = FileSystem::get_default_instance();

    log_info("waiting for sd card");

    int err = bd->init();
    log_debug("sd init (%d): %s", err, strerror(err));
    if (err != 0) {
        log_info("no sd card available");
        if (halt_on_failure)
            while (1) {}
        else 
            return 1;
    }

    log_debug("fs name: %s", fs->getName());
    fs->unmount();
    err = fs->mount(bd);
    log_debug("fs mount (%d): %s", err, strerror(err));
    
    if (err && allow_reformat) {
        log_debug("reformatting sd");
        int fs_format_err = fs->reformat(bd);
        log_debug("reformat result: %d", fs_format_err);
        if (fs_format_err) {
            log_info("unable to format sd, not able to continue");

            if (halt_on_failure)
                while (1) {}
            else 
                return 1;        
        }
    }

    return err;
}
