#include <SoftwareSerial.h>
#include <esp_ctrl.h>
#include <ir_ctrl.h>

#define NET_SSID ""
#define NET_PASS ""
#define NET_PORT "98"
#define NET_ADDR "192.168.1.10"

#define PIN_ESP_RX 3
#define PIN_ESP_TX 4
#define PIN_LED   12

#define CHECK_INTERVAL_OK  30000
#define CHECK_INTERVAL_ERR 5000

esp_ctrl esp(PIN_ESP_RX, PIN_ESP_TX, NET_SSID, NET_PASS, NET_PORT, NET_ADDR);
ir_ctrl ir;
unsigned long next_check = 0;
uint8_t offline_count = 0;

void led_on() {
  digitalWrite(PIN_LED, HIGH);
}

void led_off() {
  digitalWrite(PIN_LED, LOW);
}

void led_blink() {
  for (int i = 0; i < 2; i++) {
    led_on();
    delay(100);
    led_off();
    delay(100);
  }
}

void led_blink_on() {
  led_blink();
  led_on();
}

void handler(uint8_t id, const char* cmd, uint16_t len) {
  //Serial.print(cmd);
  if (!strncmp(cmd, "RCV", 3)) {
    //Serial.println("Receiving IR code");
    led_blink_on();
    if (ir.receive()) {
      String code = ir.code_string();
      esp.send_response(id, code);
      //Serial.println("-> DONE");
    } else {
      esp.send_response(id, "FAILED");
      //Serial.println("-> TIMEOUT");
    }
    led_blink_on();
  } else if (!strncmp(cmd, "SND ", 4)) {
    String code = cmd + 4;
    //Serial.println("Sending IR code");
    ir.send(code);
    //Serial.println("-> DONE");
    led_blink_on();
    esp.send_response(id, "OK");
  } else if (!strncmp(cmd, "X", 1)) {
    esp.send_response(id, "OK");
  } else {
    esp.send_response(id, "ERROR");
  }
  next_check = millis() + CHECK_INTERVAL_OK;
}

void error() {
  led_blink();
  next_check = millis() + 5000;
}

void check_online() {
  if (millis() > next_check) {
    if (esp.is_online()) {
      led_on();
      next_check = millis() + CHECK_INTERVAL_OK;
    } else {
      led_blink();
      offline_count++;
      if (offline_count > 30) {
        esp.init();
        offline_count = 0;
      }
      next_check = millis() + CHECK_INTERVAL_ERR;
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  esp.begin(handler, error);
  // some blinking to signal we get started
  led_blink();
  delay(1000);
  led_blink();
  while (!esp.init()) {
    // some error blinking and waiting
    for (int i = 0; i < 5; i++) {
      led_blink();
      delay(500);
    }
  }
  check_online();
}

void loop() {
  esp.loop();
  check_online();
}
