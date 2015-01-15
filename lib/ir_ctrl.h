#ifndef IR_CTRL_H_
#define IR_CTRL_H_

/*
 * Based of:
 *   https://learn.adafruit.com/ir-sensor/using-an-ir-sensor
 *   https://learn.adafruit.com/ir-sensor/making-an-intervalometer
 */

#include <Arduino.h>

/*
 * We need to use the 'raw' pin reading methods
 * because timing is very important here and the digitalRead()
 * procedure is slower!
 *
 * Digital pins 0-7 are allowed as decoder input. See:
 *
 * http://arduino.cc/en/Hacking/PinMapping168 for the 'raw' pin mapping
 * http://www.arduino.cc/en/Reference/PortManipulation
 */
#define IR_PORT_REG PIND
// Max number of intervals (ON and OFF sequences)
#define MAX_INTERVALS 100
// the maximum pulse we'll listen for - 65 milliseconds is a long time
#define MAX_PULSE 65000
// what our timing resolution should be, larger is better
// as its more 'precise' - but too large and you wont get
// accurate timing
#define RESOLUTION 20

// We have to use macros for the PINs (at least the RCV pin) as
// variable access is too slow.
#define IR_RCV_PIN 7
#define IR_SND_PIN 9

class ir_ctrl {
  public:
    ir_ctrl();
    bool receive(int8_t timeout = 10);
    String code_string() const;
    void send(String &code);

  private:
    uint16_t m_buffer[MAX_INTERVALS][2];
    uint8_t m_currentpulse;
    
    void pulse_ir(int32_t microsecs);
};
    
#endif  // IR_CTRL_H_
