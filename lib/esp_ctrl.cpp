#include "esp_ctrl.h"

bool ESP_DEBUG = false;

void esp_ctrl::begin(command_handler_t handler) {
    m_handler = handler;
    m_serial = new SoftwareSerial(m_rx, m_tx);
    m_serial->begin(9600);
}

void esp_ctrl::loop() {
    if (m_serial->available()) {
        const char *line = read_line();
        if (strlen(line) > 0) {
            if (!strncmp(line, "+IPD", 4)) {
                handle_ipd(line);
            }
        }
    }
}

bool esp_ctrl::init() {
    send_reset();
    //send_cmd_ok("AT+CWMODE=1");
    return send_cmd_ok("AT+CIPMUX=1") && send_join_ap() && send_start_server();
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
    m_serial->print(cmd);
    m_serial->print("\r\n");
}

bool esp_ctrl::send_cmd_ok(const char *cmd, int timeout) {
    send_cmd(cmd);
    return wait_for_ok(timeout);
}

bool esp_ctrl::wait_for_ok(int timeout) {
    // wait for response
    while (true) {
        const char *line = read_line(timeout);
        if (strncmp(line, "AT+", 3)) {
            return !strncmp(line, "OK", 2);
        }
    }
}

bool esp_ctrl::is_online() {
    send_cmd("AT+CIFSR");
    // wait for response
    bool online = false;
    while (true) {
        const char *line = read_line();
        if (!strncmp(line, "AT+", 3)) {
            continue;
        }
        if (atoi(line) > 0) {
            online = true;
        } else {
            return online;
        }
    }
}

void esp_ctrl::send_reset() {
    send_cmd_ok("AT+RST");
    while (strlen(read_line(2000)) > 0) {
    }
}

bool esp_ctrl::send_join_ap() {
    m_serial->print("AT+CWJAP=\"");
    m_serial->print(m_ssid);
    m_serial->print("\",\"");
    m_serial->print(m_key);
    m_serial->print("\"\r\n");
    return wait_for_ok();
}

bool esp_ctrl::send_start_server() {
    m_serial->print("AT+CIPSERVER=1,");
    m_serial->print(m_port);
    m_serial->print("\r\n");
    return wait_for_ok();
}

void esp_ctrl::send_response(uint8_t id, String &output) {
    m_serial->print("AT+CIPSEND=");
    m_serial->print(id, DEC);
    m_serial->print(",");
    m_serial->print(output.length() + 2, DEC);
    m_serial->print("\r\n");
    if (wait_for_ok()) {
        m_serial->print(output);
        m_serial->print("\r\n");
    }
}

void esp_ctrl::send_response(uint8_t id, const char *output) {
    m_serial->print("AT+CIPSEND=");
    m_serial->print(id, DEC);
    m_serial->print(",");
    m_serial->print(strlen(output) + 2, DEC);
    m_serial->print("\r\n");
    if (wait_for_ok()) {
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
