#include <SoftwareSerial.h>
#include <esp_ctrl.h>
#include <ir_ctrl.h>

#define NET_SSID ""
#define NET_PASS ""
#define NET_PORT "99"

#define PIN_ESP_RX 3
#define PIN_ESP_TX 4
#define PIN_LED   12

esp_ctrl esp(PIN_ESP_RX, PIN_ESP_TX, NET_SSID, NET_PASS, NET_PORT);
ir_ctrl ir;
unsigned long next_check = 0;

void led_on() {
  digitalWrite(PIN_LED, HIGH);
}

void led_off() {
  digitalWrite(PIN_LED, LOW);
}

void led_blink() {
  for (int i = 0; i < 5; i++) {
    led_on();
    delay(100);
    led_off();
    delay(100);
  }
}

void handler(uint8_t id, const char *cmd, uint16_t len) {
  if (!strncmp(cmd, "RCV", 3)) {
    Serial.println("Receiving IR code");
    if (ir.receive()) {
      String code = ir.code_string();
      Serial.print("Code: ");
      Serial.println(code);
      esp.send_response(id, code);
    } else {
      led_blink();
      led_on();
      Serial.println("Nothing received");
      esp.send_response(id, "FAILED");
    }
  } else if (!strncmp(cmd, "SND ", 4)) {
    String code = cmd + 4;
    Serial.println("Sending IR code");
    Serial.print("Code: ");
    Serial.println(code);
    ir.send(code);  
    esp.send_response(id, "OK");
  } else {
    esp.send_response(id, "ERROR");
  }
}

void check_online() {
  if (millis() > next_check) {
    if (esp.is_online()) {
      led_on();
    } else {
      led_blink();
    }
    // check the online state every minute
    next_check = millis() + 60000;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  esp.begin(handler);
  while (!esp.init()) {
    led_blink();
    delay(5000);
  }
  check_online();
}

void loop() {
  esp.loop();
  check_online();
}
