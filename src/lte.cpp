#include "lte.h"
#include "log.h"
#include "local_config.h"
#include "cloud_config.h"
#include "events.h"
#include "mqtt/mqtt_binary_sensor.h"
#include "mqtt/mqtt_component_discovery.h"

using namespace std::chrono_literals;

DigitalOut lte_led(LED3);
DigitalOut lte_power_pin(LTE_PWR);
DigitalOut lte_reset_pin(LTE_RST);
EventQueue lte_queue(32 * EVENTS_EVENT_SIZE);

InterruptIn green_button(PA_11);
InterruptIn red_button(PB_5);

const int lte_desired_baud = 115200;
uint8_t lte_baud_selection_counter = 0;
bool lte_baud_selection_reset = false;
lte_state_t lte_state = INIT;
BufferedSerial *lte_shield;
ATCmdParser *lte_parser;
Thread lte_thread;

Mutex lte_mutex;
Ticker lte_ticker;
bool lte_stats_send = false;

bool lte_pwr_vs_hw_reset_cycle = true;

#define LTE_BUFFER_SIZE 1024
char *lte_publish_mqtt_value_buffer = new char[LTE_BUFFER_SIZE];

uint8_t lte_error_count = 0;
uint8_t lte_operation_not_allowed_count = 0;

mqtt::MQTTBinarySensor cabin_status("cabin_status", "mdi:lan-connect", "cabin/status");

class LTESendMessage {
    protected:
        char *m_command;
        size_t m_command_size;
        char *m_expected;
        size_t m_expected_size;
        mbed::Callback<void(bool)> m_callback;
        int m_timeout = LTE_TIMEOUT;

    public:
        LTESendMessage(const char *command, const char *expected, mbed::Callback<void(bool)> callback) :
            m_callback(callback) {
            
            m_command_size = strlen(command);
            m_command = new char[m_command_size + 1];
            memcpy(m_command, command, m_command_size);
            m_command[m_command_size] = 0;

            m_expected_size = strlen(expected);
            m_expected = new char[m_expected_size + 1];
            memcpy(m_expected, expected, m_expected_size);
            m_expected[m_expected_size] = 0;
        }

        uint8_t retry_count = 0;

        ~LTESendMessage() {
            delete[] m_command;
            delete[] m_expected;
        }
        
        const char* get_command() { 
            return m_command;
        }

        const char* get_expected() { 
            return m_expected;
        }

        mbed::Callback<void(bool)> get_command_callback() {
            return m_callback;
        }

        int get_timeout() {
            return m_timeout;
        }

        void set_timeout(int timeout) {
            m_timeout = timeout;
        }
};

Queue<LTESendMessage, 16> lte_send_queue;
Thread lte_send_thread; 

void lte_send_thread_handler() {
    while (true) {
        LTESendMessage *msg;
        if (lte_send_queue.try_get(&msg)) {
            if (lte_state == READY) {
                log_debug("got from queue, count %d: %s (%p) on ctx %p", msg->retry_count, msg->get_command(), msg, ThisThread::get_id());

                lte_mutex.lock();

                bool result = false;
                lte_parser->set_timeout(msg->get_timeout());

                while (!result && msg->retry_count++ < 3) {
                    result = lte_parser->send(msg->get_command()) && lte_parser->recv(msg->get_expected());

                    if (!result) ThisThread::sleep_for(2s);
                }

                lte_parser->set_timeout(LTE_TIMEOUT);

                mbed::Callback<void(bool)> _cb = msg->get_command_callback();
                if (_cb) {
                    _cb(result);
                }

                delete msg;
                lte_mutex.unlock();

                if (!result && msg->retry_count < 5) {
                    log_debug("retrying message, count: %d", msg->retry_count);
                    //lte_send_queue.try_put(msg);
                }
            } else {
                mbed::Callback<void(bool)> _cb = msg->get_command_callback();
                if (_cb) {
                    _cb(false);
                }
            }
        }
    }
}

void lte_flip_send_bit() {
    lte_stats_send = true;
}

void green_button_run() {
    log_debug("green button pressed");
    lte_mutex.lock();
    lte_parser->process_oob();
    lte_mutex.unlock();
}

void green_button_pressed() {
    if (lte_state == READY)
        lte_queue.call(green_button_run);
}

void red_button_pressed() {
    lte_parser->abort();
    lte_queue.call(lte_mqtt_login);
}

void lte_init() {
    green_button.mode(PullDown);
    green_button.rise(&green_button_pressed);

    red_button.mode(PullDown);
    red_button.rise(&red_button_pressed);

    lte_led = 0;
    if (lte_shield != NULL) {
        delete lte_shield;
    }

    if (lte_parser != NULL) {
        delete lte_parser;
    }

    log_debug("initializing lte shield on cxt %p", ThisThread::get_id());
    lte_shield = new BufferedSerial(LTE_TX, LTE_RX, lte_desired_baud);   
    lte_shield->set_flow_control(BufferedSerial::Flow::Disabled); 
    lte_shield->set_format(8, BufferedSerial::Parity::None, 1);

    lte_parser = new ATCmdParser(lte_shield, "\r", LTE_BUFFER_SIZE);
    lte_parser->debug_on(true);
    lte_parser->set_timeout(LTE_TIMEOUT);
    lte_parser->flush();

    lte_power_pin = 1;
    lte_reset_pin = 1;

    lte_send_thread.start(callback(lte_send_thread_handler));
    lte_thread.start(callback(&lte_queue, &EventQueue::dispatch_forever));
    lte_queue.call(lte_discover_baud_rate);

    mqtt::mqtt_register_component(&cabin_status);
    lte_ticker.attach(callback(lte_flip_send_bit), cloud_config()->lte_interval);
}

void lte_reset_tasks() {
    lte_led = 0;
    lte_parser->abort();
    lte_error_count = 0;
    lte_operation_not_allowed_count = 0;
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

void lte_cycle() {
    if (lte_pwr_vs_hw_reset_cycle) {
        lte_power_cycle();
    } else {
        lte_hw_reset();
    }

    lte_pwr_vs_hw_reset_cycle = !lte_pwr_vs_hw_reset_cycle;
}

void lte_discover_baud_rate() {
    lte_reset_tasks();

    lte_mutex.lock();

    lte_state = BAUD_SELECTION;
    events_clear(FLAG_SYSTEM_READY);

    bool success = false;
    lte_parser->set_timeout(500);
    while (!success) {
        for (int i=0; i<NUM_SUPPORTED_BAUD; i++) {
            lte_shield->set_baud(LTE_SHIELD_SUPPORTED_BAUD[i]);
            if (lte_parser->send("AT+IPR=%d", lte_desired_baud)) {
                ThisThread::sleep_for(500ms);
                lte_shield->set_baud(lte_desired_baud);
                ThisThread::sleep_for(200ms);

                lte_parser->flush();
                if (lte_parser->send("ATE0") && lte_parser->recv("OK")) {
                    log_debug("serial ready on cxt %p", ThisThread::get_id());
                    success = true;
                    break;
                }
            }
        }

        if (!success) {
            lte_cycle();
        }
    }

    log_debug("autobaud selected: %d on cxt %p", lte_desired_baud, ThisThread::get_id());
    if (success) {
        lte_queue.call(lte_operator_registration);
    } else {
        lte_queue.call(lte_discover_baud_rate);
    }

    lte_parser->set_timeout(LTE_TIMEOUT);
    lte_mutex.unlock();
}

void lte_operator_registration() {
    lte_reset_tasks();

    lte_mutex.lock();

    lte_state = OPERATOR_REGISTRATION;
    events_clear(FLAG_SYSTEM_READY);

    log_debug("operator registration staring on cxt %p", ThisThread::get_id());
    lte_parser->set_timeout(5000);

    // are we already online and registered?
    if (lte_parser->send("AT+CEREG?")) {
        int eps_status;
        if (!lte_parser->recv("+CEREG: %*d,%d", &eps_status)) {
            lte_cycle();
            lte_queue.call(lte_discover_baud_rate);
            lte_mutex.unlock();
            return;
        }

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
   
#ifdef LTE_AUTO_DISCOVERY
    bool registered = false;
    bool searching = true;
    uint16_t counter = 0;

    while (searching) {
        lte_led = 1;
        if (lte_parser->send("AT+CEREG?")) {
            int eps_status;

            if (!lte_parser->recv("+CEREG: %*d,%d", &eps_status)) {
                lte_led = 0;
                lte_cycle();
                lte_queue.call(lte_discover_baud_rate);
                lte_mutex.unlock();
                return;
            }

            counter++;

            switch (eps_status) {
                case 1:
                case 5:
                    log_debug("network registration completed after %d attempts", counter);
                    registered = true;
                    searching = false;
                    break;
                
                case 2:
                case 6:
                    log_debug("still performing network registration, attempt %d/unlimited", counter);
                    break;

                default:
                    registered = false;
                    searching = true;
                    break;
            }
        }

        lte_led = 0;
        if (searching) {
            ThisThread::sleep_for(10s);
        }
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

                lte_parser->set_timeout(LTE_TIMEOUT);
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

void lte_issue_read_messages_request() {
    lte_parser->remove_oob(LTE_MQTT_UNREAD_MESSAGES);

    if (!lte_mutex.trylock_for(60s)) {
        log_debug("skipping read messages, another using it?");
        return;
    }

    log_debug("read messages");
    lte_parser->set_timeout(120000);
    if (lte_parser->send("AT+UMQTTC=6")) {
        char buffer[256] = {0};
        size_t buffer_idx = 0;
        size_t seperator_idx = 1;
        const char *seperator = "\r\n";

        bool completed = false;
        
        char topic[32] = {0};
        char value[256] = {0};

        while (!completed) {
            char c = lte_parser->getc();

            bool found = false;
            if (c == seperator[seperator_idx]) {
                seperator_idx++;
                if (seperator_idx >= strlen(seperator)) {
                    seperator_idx = 0;
                    found = true;
                }
            }

            if (found) {
                int mqtt_result;
                if (sscanf(buffer, "+UMQTTC: 6,%d", &mqtt_result) == 1) {
                    switch (mqtt_result) {
                        case 1:
                            log_debug("read messages completed successfully");
                            break;
                        
                        default:
                            log_debug("read messages failed");
                    }

                    completed = true;
                } else if (sscanf(buffer, "Topic:%s", topic) == 1) {
                } else if (sscanf(buffer, "Msg:%[^\r\n]", value) == 1) {
                    log_debug("setting %s to %s", topic, value);

                    if (strcmp("cabin/config", topic) == 0) {
                        std::string json(value);
                        cloud_config_set(json);
                    }

                    memset(topic, 0, sizeof(topic));
                    memset(value, 0, sizeof(value));
                }

                memset(buffer, 0, sizeof(buffer));
                buffer_idx = 0;
            } else {
                buffer[buffer_idx++] = c;
            }
        }
    }

    log_debug("exiting read messages");
    lte_parser->set_timeout(LTE_TIMEOUT);
    lte_mutex.unlock();
    lte_parser->oob(LTE_MQTT_UNREAD_MESSAGES, lte_issue_read_messages_request);
}

void lte_handle_error() {
    lte_error_count++;
    lte_parser->send("AT+UMQTTER");
    int err_code, secondary_err_code;
    lte_parser->recv("+UMQTTER: %d,%d", &err_code, &secondary_err_code);
    log_debug("mqtt error information: %d,%d", err_code, secondary_err_code);

    if (lte_state != LTE_ERROR && lte_error_count > LTE_ERROR_LIMIT) {
        lte_state = LTE_ERROR;
        lte_parser->abort();
        lte_queue.call(lte_mqtt_login);
    }
}

void lte_handle_op_not_allowed() {
    lte_operation_not_allowed_count++;

    if (lte_state != LTE_ERROR && lte_operation_not_allowed_count > LTE_OP_NOT_ALLOWED_LIMIT) {
        lte_state = LTE_ERROR;
        lte_parser->abort();
        lte_queue.call(lte_mqtt_login);
    }
}

void lte_handle_login_failed() {
    lte_led = 0;
    ThisThread::sleep_for(2s);
    lte_parser->abort();
}

void lte_mqtt_login() {
    // reset state for mqtt
    lte_reset_tasks();
    lte_parser->remove_oob(LTE_MQTT_URC_LOGGED_OUT);
    lte_parser->remove_oob(LTE_MQTT_LOGIN_FAILED);
    lte_parser->remove_oob(LTE_MQTT_OP_NOT_SUPPORTED);
    lte_parser->remove_oob(LTE_MQTT_ERROR);
    lte_parser->remove_oob(LTE_MQTT_UNREAD_MESSAGES);

    lte_mutex.lock();

    lte_state = MQTT_LOGIN;
    events_clear(FLAG_SYSTEM_READY);

    log_debug("mqtt login on cxt %p", ThisThread::get_id());

    // handle failures
    lte_parser->oob(LTE_MQTT_LOGIN_FAILED, lte_handle_login_failed);
    lte_parser->oob(LTE_MQTT_OP_NOT_SUPPORTED, lte_handle_op_not_allowed);
    lte_parser->oob(LTE_MQTT_OP_NOT_ALLOWED, lte_handle_op_not_allowed);

    bool setup_result = false;
    // mqtt configuration
    lte_parser->set_timeout(1000);
    if (lte_parser->send("AT+UMQTTC=0") && lte_parser->recv("OK") && // mqtt logout
        lte_parser->send("AT+UMQTT=12,1") && lte_parser->recv("OK") && // mqtt clear session
        lte_parser->send("AT+UMQTT=0,\"cabin\"") && lte_parser->recv("+UMQTT: 0,1") && // set mqtt client id
        lte_parser->send("AT+UMQTT=2,\"%s\",1883", local_get_config_mqtt_hostname()) && lte_parser->recv("+UMQTT: 2,1") && // mqtt connection information
        lte_parser->send("AT+UMQTT=10,30") && lte_parser->recv("+UMQTT: 10,1") && // set mqtt inactivity timeout
        lte_parser->send("AT+UMQTTWTOPIC=1,1,\"cabin/status\"") && lte_parser->recv("OK") && // mqtt last will topic
        lte_parser->send("AT+UMQTTWMSG=\"OFF\"") && lte_parser->recv("OK") // mqtt last will message
    ) {
        // ready to go!
        setup_result = true;
    }

    // mqtt login and status
    lte_parser->set_timeout(60000);
    if ( setup_result &&
        lte_parser->send("AT+UMQTTC=1") && lte_parser->recv("+UMQTTC: 1,1") // mqtt login
    ) {
        ThisThread::sleep_for(2s);

        // handle being logged out (i.e. lost the server)
        lte_parser->oob(LTE_MQTT_URC_LOGGED_OUT, lte_handle_login_failed);

        // handle expected errors
        lte_parser->oob(LTE_MQTT_ERROR, lte_handle_error);

        // handle published messages
        lte_parser->oob(LTE_MQTT_UNREAD_MESSAGES, lte_issue_read_messages_request);

        // subscribe to configuration data
        if (lte_parser->send("AT+UMQTTC=4,1,\"cabin/config\"") && lte_parser->recv("+UMQTTC: 4,1")) {
            
            log_debug("subscribed to cabin/config");
        }

        lte_parser->set_timeout(LTE_TIMEOUT);
        lte_mutex.unlock();
        log_debug("mqtt setup completed, we're ready to go on cxt %p", ThisThread::get_id());
        lte_state = READY;

        cabin_status.publish_state(true, [](bool result){
            if (result) {
                events_set(FLAG_SYSTEM_READY);
                lte_led = 1;
            } else {
                log_debug("not able to publish cabin state online");
                lte_queue.call(lte_operator_registration);
            }
        });
    } else {
        // sleep for a bit
        ThisThread::sleep_for(3s);
        lte_queue.call(lte_operator_registration);
        lte_parser->set_timeout(LTE_TIMEOUT);
        lte_mutex.unlock();
    }
}

void _send(const char *command, const char *expected_response, mbed::Callback<void(bool)> _cb) {
    lte_mutex.lock();

    log_debug("sending command on ctx %p", ThisThread::get_id());
    bool result = lte_parser->send(command) && lte_parser->recv(expected_response);

    if (_cb != NULL)
        _cb(result);

    lte_mutex.unlock();
}

bool lte_send(const char *command, const char *expected_response, mbed::Callback<void(bool)> _cb, int timeout) {
    if (lte_state != READY) {
        log_debug("send failed, lte_state (%d) not ready", lte_state);
        return false;
    }

    LTESendMessage *msg = new LTESendMessage(command, expected_response, _cb);
    msg->set_timeout(timeout);

    bool result = lte_send_queue.try_put(msg);
    return result;
}

bool lte_publish(const char *topic, const char *value, mbed::Callback<void(bool)> _cb, int timeout, bool retain, ...) {
    va_list vl;
    va_start(vl, retain);
    bool result = lte_vpublish(topic, value, _cb, timeout, retain, vl);
    va_end(vl);

    return result;
}

bool lte_vpublish(const char *topic, const char *value, mbed::Callback<void(bool)> _cb, int timeout, bool retain, va_list args) {
    memset(lte_publish_mqtt_value_buffer, 0, LTE_BUFFER_SIZE);
    vsprintf(lte_publish_mqtt_value_buffer, value, args);

    char *command = new char[strlen(topic) + strlen(lte_publish_mqtt_value_buffer) + 30];
    sprintf(command, "AT+UMQTTC=2,0,%d,\"%s\",\"%s\"", retain, topic, lte_publish_mqtt_value_buffer);

    bool result = lte_send(command, "+UMQTTC: 2,1", _cb, timeout);

    delete[] command;
    return result;
}

void lte_send_stats() {
    if (lte_state == READY) {
        lte_mutex.lock();
    
        int rssi, qual;
        if (lte_parser->send("AT+CSQ") && lte_parser->recv("+CSQ: %d,%d", &rssi, &qual)) {

            lte_publish("cabin/lte/rssi", "%d", NULL, 1000, true, rssi);
            lte_publish("cabin/lte/quality", "%d", NULL, 1000, true, qual);
        }
        
        lte_mutex.unlock();    
    }

    lte_ticker.attach(callback(lte_flip_send_bit), cloud_config()->lte_interval);
}


void lte_loop() {
    if (lte_stats_send) {
        lte_stats_send = false;
        lte_ticker.detach();
        lte_queue.call(lte_send_stats);
    }
}

void lte_oob_loop() {
    if (lte_mutex.trylock_for(100ms)) {
        lte_parser->debug_on(false);
        lte_parser->process_oob();
        lte_parser->debug_on(true);
        lte_mutex.unlock();
    }
}