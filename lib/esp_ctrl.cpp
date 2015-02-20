#include "esp_ctrl.h"

bool ESP_DEBUG = false;

void esp_ctrl::begin(command_handler_t handler, error_handler_t error) {
    m_handler = handler;
    m_err_handler = error;
    m_serial = new SoftwareSerial(m_rx, m_tx);
    m_serial->begin(9600);
}

void esp_ctrl::loop() {
    if (m_serial->available()) {
        const char *line = read_line();
        if (*line != 0) {
            if (!strncmp(line, "+IPD", 4)) {
                if (ESP_DEBUG) {
                    Serial.println("<< REQUEST");
                }
                handle_ipd(line);
            } else if (!strncmp(line, "Unlink", 4)) {
                if (!connect()) {
                    m_err_handler();
                }
            }
        }
    }
}

bool esp_ctrl::init() {
    reset();
    send_cmd("AT+CWMODE=1");
    wait_for2("OK", "no change");
    return send_cmd_ok("AT+CIPMUX=1") && join_ap() && connect();
}

const char *esp_ctrl::read_line(int timeout) {
    uint32_t now = millis();
    uint32_t future = now + timeout;
    if (future < now) {
        // overflow
        delay(timeout);
        now = millis();
        future = now + timeout;
    }
    uint16_t len = sizeof(m_buffer);
    uint16_t i = 0;
    while (i < len - 1 && now < future) {
        if (m_serial->available()) {
            char c = m_serial->read();
            if (ESP_DEBUG) {
                Serial.write(c);
            }
            m_buffer[i++] = c;
            if (i > 1 && m_buffer[i - 2] == '\r' && m_buffer[i - 1] == '\n') {
                if (i == 2) {
                    i = 0;  // skip empty lines
                } else {
                    m_buffer[i - 2] = 0;
                    return m_buffer;
                }
            }
        }
        now = millis();
    }
    m_buffer[i] = 0;
    return m_buffer;
}

void esp_ctrl::send_cmd(const char *cmd) {
    if (ESP_DEBUG) {
        Serial.print(">> ");
        Serial.println(cmd);
    }
    m_serial->print(cmd);
    m_serial->print("\r\n");
}

bool esp_ctrl::send_cmd_ok(const char *cmd, int timeout) {
    send_cmd(cmd);
    return wait_for_ok(timeout);
}

bool esp_ctrl::wait_for(const char* what, int timeout) {
    // wait for response
    while (true) {
        const char *line = read_line(timeout);
        if (*line == 0) {
            return false;
        }
        if (!strncmp(line, what, strlen(what))) {
            return true;
        }
    }
}

bool esp_ctrl::wait_for2(const char* what1, const char* what2, int timeout) {
    // wait for response
    while (true) {
        const char* line = read_line(timeout);
        if (*line == 0) {
            return false;
        }
        if (!strncmp(line, what1, strlen(what1)) || !strncmp(line, what2, strlen(what2))) {
            return true;
        }
    }
}

bool esp_ctrl::wait_for_else(const char* expected, const char* not_expected, int timeout) {
    // wait for response
    while (true) {
        const char* line = read_line(timeout);
        if (*line == 0) {
            return false;
        }
        if (!strncmp(line, expected, strlen(expected))) {
            return true;
        }
        if (!strncmp(line, not_expected, strlen(not_expected))) {
            return false;
        }
    }
}

bool esp_ctrl::wait_for_ok(int timeout) {
    // wait for response
    return wait_for("OK", timeout);
}

int esp_ctrl::wait_for_status(int timeout) {
    // wait for response
    while (true) {
        const char *line = read_line(timeout);
        if (*line == 0) {
            return 0;
        }
        if (!strncmp(line, "STATUS:", 7)) {
            return atoi(line + 7);
        }
    }    
}

bool esp_ctrl::is_online() {
    send_cmd("AT+CIPSTATUS");
    int status = wait_for_status();
    wait_for("OK");
    if (3 != status) {
        return connect();
    }
    return true;
    
    //m_serial->print("AT+CIPSTART=4,\"TCP\",\"");
    //m_serial->print(m_chk_addr);
    //m_serial->print("\",");
    //m_serial->print(m_chk_port);
    //m_serial->print("\r\n");
    //if (wait_for("Linked")) {
    //    send_cmd("AT+CIPCLOSE=4");
    //    wait_for("Unlink");
    //    return true;
    //} else {
    //    return false;
    //}

    //send_cmd("AT+CIFSR");
    //// wait for response
    //bool online = false;
    //while (true) {
    //    const char *line = read_line();
    //    if (!strncmp(line, "AT+", 3)) {
    //        continue;
    //    }
    //    if (atoi(line) > 0) {
    //        online = true;
    //    } else {
    //        return online;
    //    }
    //}
}

void esp_ctrl::reset() {
    send_cmd("AT+RST");
    wait_for("ready");
}

bool esp_ctrl::join_ap() {
    if (ESP_DEBUG) {
        Serial.println(">> AT+CWJAP=...");
    }
    m_serial->print("AT+CWJAP=\"");
    m_serial->print(m_ssid);
    m_serial->print("\",\"");
    m_serial->print(m_key);
    m_serial->print("\"\r\n");
    return wait_for_ok(10000);
}

/*
bool esp_ctrl::send_start_server() {
    if (ESP_DEBUG) {
        Serial.println(">> AT+CIPSERVER=1,...");
    }
    m_serial->print("AT+CIPSERVER=1,");
    m_serial->print(m_port);
    m_serial->print("\r\n");
    return wait_for2("OK", "no change");
}
*/

bool esp_ctrl::connect() {
    if (ESP_DEBUG) {
        Serial.println(">> AT+CIPSTART=4,\"TCP\",...");
    }
    m_serial->print("AT+CIPSTART=4,\"TCP\",\"");
    m_serial->print(m_addr);
    m_serial->print("\",");
    m_serial->print(m_port);
    m_serial->print("\r\n");
    if (wait_for_else("Linked", "Unlink")) {
        return true;
    } else {
        return false;
    }
}

void esp_ctrl::send_response(uint8_t id, String &output) {
    if (ESP_DEBUG) {
        Serial.println(">> AT+CIPSEND=...");
    }
    m_serial->print("AT+CIPSEND=");
    m_serial->print(id, DEC);
    m_serial->print(",");
    m_serial->print(output.length() + 2, DEC);
    m_serial->print("\r\n");
    if (wait_for_ok()) {
        if (ESP_DEBUG) {
            Serial.println(">> DATA...");
        }
        m_serial->print(output);
        m_serial->print("\r\n");
    }
}

void esp_ctrl::send_response(uint8_t id, const char *output) {
    if (ESP_DEBUG) {
        Serial.println(">> AT+CIPSEND=...");
    }
    m_serial->print("AT+CIPSEND=");
    m_serial->print(id, DEC);
    m_serial->print(",");
    m_serial->print(strlen(output) + 2, DEC);
    m_serial->print("\r\n");
    if (wait_for_ok()) {
        if (ESP_DEBUG) {
            Serial.println(">> DATA...");
        }
        m_serial->print(output);
        m_serial->print("\r\n");
    }
}

void esp_ctrl::handle_ipd(const char *input) {
    uint8_t id = 0;
    uint16_t len = 0;
    uint16_t inp_len = strlen(input);
    if (inp_len < 6) {
        return;
    }
    const char *s = input + 5;
    const char *e = strchr(s, ',');
    if (NULL == e) {
        return;
    }
    id = atoi(s);
    s = e + 1;
    e = strchr(s, ':');
    if (NULL == e) {
        return;
    }
    len = atoi(s);
    s = e + 1;
    m_handler(id, s, len - 1);
}
