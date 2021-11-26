#include "cli.h"
#include "mbed.h"
#include "rtc.h"

static BufferedSerial pc(USBTX, USBRX, 115200);
EventQueue cli_queue(32 * EVENTS_EVENT_SIZE);
Thread cli_thread;

int fuck = 0;

void cli_time() {
    rtc_time_t t;
    if (rtc_read_time(&t) == 0) {
        printf("time: %02d/%02d/%d %02d:%02d:%02d\n\n", t.month, t.date, t.year, t.hours, t.minute, t.second);
    } else {
        printf("rtc failed\n\n");
    }

}

void cli_help() {
    printf("CLI Help\n\n");
    printf("h:   help\n");
    printf("T:   set time\n");
    printf("t:   show time\n");
    printf("M:   show mqtt host\n");
    printf("m:   set mqtt host\n\n");
}

void cli_handle() {
    if (pc.readable()) {
        char c;
        pc.read(&c, 1);
    
        switch (c) {
            case 'h':
                cli_help();
                break;
            
            case 't':
                cli_time();
                break;

            default:
                printf("unknown command: %c\n", c);
                break;
        }
    }
}

void cli_sigio_handler() {
    cli_queue.call(cli_handle);
}

void cli_init() {
    pc.set_blocking(false);
    pc.sigio(callback(cli_sigio_handler));
    cli_thread.start(callback(&cli_queue, &EventQueue::dispatch_forever));
    const char *msg = "\n\nCLI Ready (h for help)\n\n";
    pc.write(msg, sizeof(msg));
    pc.sync();
}