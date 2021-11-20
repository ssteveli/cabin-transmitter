#include "lte.h"
#include "log.h"
#include "config.h"

using namespace std::chrono_literals;

DigitalOut lte_power_pin(D5);
DigitalOut lte_reset_pin(D6);
EventQueue lte_queue(32 * EVENTS_EVENT_SIZE);

const int lte_desired_baud = 9600;
uint8_t lte_baud_selection_counter = 0;
bool lte_baud_selection_reset = false;
lte_state_t lte_state = INIT;
BufferedSerial *lte_shield;
ATCmdParser *lte_parser;
Thread lte_thread;

Mutex lte_mutex;
Mutex lte_read_messages_mutex;

void lte_init() {
    log_debug("initializing lte shield on cxt %p", ThisThread::get_id());
    lte_shield = new BufferedSerial(PA_9, PA_10, lte_desired_baud);   
    lte_shield->set_flow_control(BufferedSerial::Flow::Disabled); 
    lte_shield->set_format(8, BufferedSerial::Parity::None, 1);

    lte_parser = new ATCmdParser(lte_shield, "\r");
    lte_parser->debug_on(true);
    lte_parser->set_timeout(1000);
    lte_parser->flush();

    lte_power_pin = 1;
    lte_reset_pin = 1;

    lte_thread.start(callback(&lte_queue, &EventQueue::dispatch_forever));
    lte_queue.call(lte_discover_baud_rate);
}

void lte_power_cycle() {
    log_debug("power cycling shield on cxt %p", ThisThread::get_id());
    lte_power_pin = 0;
    wait_us(3200000);
    lte_power_pin = 1;
}

void lte_hw_reset() {
    log_debug("hw reset shield on cxt %p", ThisThread::get_id());
    lte_reset_pin = 0;
    wait_us(10000000);
    lte_reset_pin = 1;
}

void lte_discover_baud_rate() {
    lte_mutex.lock();

    lte_state = BAUD_SELECTION;

    bool success = false;
    bool pwr = true;
    while (!success) {
        for (int i=0; i<NUM_SUPPORTED_BAUD; i++) {
            lte_shield->set_baud(LTE_SHIELD_SUPPORTED_BAUD[i]);
            if (lte_parser->send("AT+IPR=%d", lte_desired_baud)) {
                lte_shield->set_baud(lte_desired_baud);
                wait_us(200000);

                lte_parser->flush();
                if (lte_parser->send("ATE0") && lte_parser->recv("OK")) {
                    log_debug("serial ready on cxt %p", ThisThread::get_id());
                    success = true;
                    break;
                }
            }
        }

        if (!success) {
            if (pwr) {
                lte_power_cycle();
            } else {
                lte_hw_reset();
            }

            pwr = !pwr;
        }
    }

    log_debug("autobaud selected: %d on cxt %p", lte_desired_baud, ThisThread::get_id());
    if (success) {
        lte_queue.call(lte_operator_registration);
    } else {
        lte_queue.call(lte_discover_baud_rate);
    }

    lte_mutex.unlock();
}

void lte_operator_registration() {
    lte_mutex.lock();

    lte_state = OPERATOR_REGISTRATION;

    log_debug("operator registration staring on cxt %p", ThisThread::get_id());
    lte_parser->set_timeout(5000);

#define AUTO_DISCOVERY

    // are we already online and registered?
    if (lte_parser->send("AT+CEREG?")) {
        int eps_status;
        lte_parser->recv("+CEREG: %*d,%d", &eps_status);

        if (eps_status == 1 || eps_status == 5) {
            lte_queue.call(lte_mqtt_login);
            lte_mutex.unlock();
            return;
        } else if (!(lte_parser->send("AT+UMNOPROF=%d", MOBILE_NETWORK_OPERATOR) && lte_parser->recv("OK")) ||
                   !(lte_parser->send("AT+CGDCONT=1,\"IP\",\"%s\"", APN) && lte_parser->recv("OK")) ||
                   !(lte_parser->send("AT+COPS=0") && lte_parser->recv("OK"))) {
                
                lte_queue.call(lte_discover_baud_rate);
                lte_mutex.unlock();
                return;
        }
    } else {
        lte_queue.call(lte_discover_baud_rate);
    }
   
#ifdef AUTO_DISCOVERY
    bool registered = false;
    for (int i=0; i<50; i++) {
        if (lte_parser->send("AT+CEREG?")) {
            int eps_status;
            lte_parser->recv("+CEREG: %*d,%d", &eps_status);

            if (eps_status == 1 || eps_status == 5) {
                registered = true;
                break;
            } else if (eps_status == 2) {
                log_debug("still performing network registration, attempt %d/%d", i, 50);
            } else {
                lte_hw_reset();
                registered = false;
                break;
            }
        }

        wait_us(10000000);
    }

    if (registered) {
        lte_queue.call(lte_mqtt_login);
    } else {
        lte_queue.call(lte_discover_baud_rate);
    }
#endif

#ifdef MANUAL_DISCOVERY
    int mode;
    char oper[24];
    oper[0] = '\0';

    if (lte_parser->send("AT+COPS?")) {
        bool oper_check = lte_parser->recv("+COPS: %d,%*[^,],\"%[^\"]\",%*d", &mode, oper);

        if (!oper_check || strlen(oper) == 0) {
            log_debug("setup for scanning operators on cxt %p", ThisThread::get_id());
            if (lte_parser->send("AT+UMNOPROF=%d", MOBILE_NETWORK_OPERATOR) && lte_parser->recv("OK") &&
                lte_parser->send("AT+CGDCONT=1,\"IP\",\"%s\"", APN) && lte_parser->recv("OK")) {

                if (lte_parser->send("AT+COPS=0") && lte_parser->recv("OK")) {
                    bool registered = false;
                    for (int i=0; i<5; i++) {
                        if (lte_parser->send("AT+COPS?")) {
                            bool oper_check = lte_parser->recv("+COPS: %d,%*[^,],\"%[^\"]\",%*d", &mode, oper);
                            if (!oper_check || strlen(oper) == 0) {
                                wait_us(10000000);
                            } else {
                                registered = true;
                                break;
                            }
                        }
                    }

                    if (registered) {
                        lte_queue.call(lte_mqtt_login);
                    } else {
                        lte_queue.call(lte_discover_baud_rate);
                    }
                } else {
                    log_debug("failure scanning for operators on cxt %p", ThisThread::get_id());
                    lte_queue.call(lte_discover_baud_rate);
                }

                lte_parser->set_timeout(1000);
            }
        } else {
            lte_queue.call(lte_mqtt_login);
        }
    } else {
        lte_queue.call(lte_discover_baud_rate);
    }
#endif

    lte_mutex.unlock();
}

void lte_uumqtcc() {
    log_debug("lte_uumqtcc called");
    lte_read_messages_mutex.unlock();
}

void lte_issue_read_messages_request() {
    if (!lte_read_messages_mutex.trylock()) {
        return;
    }

    lte_mutex.lock();
    lte_parser->remove_oob("+UUMQTTC: 6,");

    log_debug("read messages");
    if(lte_parser->send("AT+UMQTTC=6") && lte_parser->recv("+UMQTTC: 6,1")) {
        lte_parser->oob("+UUMQTTC: 6,", lte_uumqtcc);
    } else {
        lte_read_messages_mutex.unlock();
    }

    lte_mutex.unlock();
}

void lte_umqttc_config() {
    log_debug("reading configuration for config accepted");
}

void lte_mqtt_login() {
    lte_mutex.lock();

    lte_state = MQTT_LOGIN;
    log_debug("mqtt login on cxt %p", ThisThread::get_id());

    lte_parser->set_timeout(60000);
    if (lte_parser->send("AT+UMQTTC=0") && lte_parser->recv("OK") && // mqtt logout
        lte_parser->send("AT+UMQTT=12,1") && lte_parser->recv("OK") && // mqtt clear session
        lte_parser->send("AT+UMQTT=0,\"cabin\"") && lte_parser->recv("+UMQTT: 0,1") && // set mqtt client id
        lte_parser->send("AT+UMQTT=2,\"%s\",1883", get_config_mqttt_hostname()) && lte_parser->recv("+UMQTT: 2,1") && // mqtt connection information
        lte_parser->send("AT+UMQTT=10,3600") && lte_parser->recv("+UMQTT: 10,1") && // set mqtt inactivity timeout
        lte_parser->send("AT+UMQTTWTOPIC=1,1,\"cabin/status\"") && lte_parser->recv("OK") && // mqtt last will topic
        lte_parser->send("AT+UMQTTWMSG=\"off\"") && lte_parser->recv("OK") && // mqtt last will message
        lte_parser->send("AT+UMQTTC=1") && lte_parser->recv("+UMQTTC: 1,1") && // mqtt login
        lte_parser->send("AT+UMQTTC=2,0,0,\"cabin/status\",\"on\"") && lte_parser->recv("+UMQTTC: 2,1") // set cabin status to on
    ) {
        wait_us(2000000);
        lte_parser->oob("+UUMQTTC: 4,1,0,\"cabin/config\"", lte_umqttc_config);

        if (lte_parser->send("AT+UMQTTC=4,0,\"cabin/config\"") && lte_parser->recv("+UMQTTC: 4,1")) {}

        // schedule a refresh to read unread URC messages
        lte_queue.call_every(60s, lte_issue_read_messages_request);

        log_debug("mqtt setup completed, we're ready to go on cxt %p", ThisThread::get_id());
        lte_state = READY;
    } else {
        lte_queue.call(lte_operator_registration);
    }

    lte_parser->set_timeout(1000);
    lte_mutex.unlock();
}

void _send(const char *command, const char *expected_response, mbed::Callback<void(bool)> _cb) {
    lte_mutex.lock();

    log_debug("sending command on ctx %p", ThisThread::get_id());
    bool result = lte_parser->send(command) && lte_parser->recv(expected_response);

    if (_cb != NULL)
        _cb(result);

    lte_mutex.unlock();
}

bool lte_send(const char *command, const char *expected_response, mbed::Callback<void(bool)> _cb) {
    if (lte_state != READY) {
        log_debug("send failed, lte_state (%d) not ready", lte_state);
        return false;
    }

    return lte_queue.call(_send, command, expected_response, _cb);
}

bool lte_publish(const char *topic, const char *value, mbed::Callback<void(bool)> _cb) {
    if (lte_state != READY) {
        log_debug("publish failed, lte_state (%d) not ready", lte_state);
        return false;
    }

    std::string command = "AT+UMQTTC=2,0,0,\"" + std::string(topic) + "\",\"" + std::string(value) + "\"";

    return lte_queue.call(_send, command.c_str(), "+UMQTTC: 2,1", _cb) > 0;
}