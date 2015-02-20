#ifndef ESP_CTRL_H_
#define ESP_CTRL_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

// reduce this, if you need more memory for other stuff
#define MAX_LINE_LEN 512

class esp_ctrl {
  public:
    typedef void (*command_handler_t)(uint8_t id, const char *cmd, uint16_t len);
    typedef void (*error_handler_t)();

    esp_ctrl(uint8_t rx, uint8_t tx, const char* ssid, const char* key, const char* port, const char* addr)
        : m_rx(rx),
          m_tx(tx),
          m_serial(NULL),
          m_ssid(ssid),
          m_key(key),
          m_port(port),
          m_addr(addr),
          m_handler(NULL) {}
    void begin(command_handler_t handler, error_handler_t error);
    bool init();
    void loop();
    bool is_online();
    void send_response(uint8_t id, const char *output);
    void send_response(uint8_t id, String &output);

  private:
    uint8_t m_rx;
    uint8_t m_tx;
    SoftwareSerial *m_serial;
    const char* m_ssid;
    const char* m_key;
    const char* m_port;
    const char* m_addr;

    command_handler_t m_handler;
    error_handler_t m_err_handler;

    char m_buffer[MAX_LINE_LEN];
    
    const char *read_line(int timeout = 1000);
    void send_cmd(const char* cmd);
    bool send_cmd_ok(const char* cmd, int timeout = 5000);
    bool wait_for(const char* what, int timeout = 3000);
    bool wait_for2(const char* what1, const char* what2, int timeout = 3000);
    bool wait_for_else(const char* expected, const char* not_expected, int timeout = 3000);
    bool wait_for_ok(int timeout = 3000);
    int wait_for_status(int timeout = 3000);
    void reset();
    bool join_ap();
    bool connect();
    void handle_ipd(const char *input);
};

#endif  // ESP_CTRL_
