#include "ir_ctrl.h"

ir_ctrl::ir_ctrl() {
    pinMode(IR_RCV_PIN, INPUT);
    pinMode(IR_SND_PIN, OUTPUT);
    digitalWrite(IR_SND_PIN, LOW);
}


// This procedure sends a 38KHz pulse to the IR_SND_PIN 
// for a certain # of microseconds. We'll use this whenever we need to send codes
void ir_ctrl::pulse_ir(int32_t microsecs) {
    cli();  // this turns off any background interrupts

    while (microsecs > 0) {
        // 38 kHz is about 13 microseconds high and 13 microseconds low
        digitalWrite(IR_SND_PIN, HIGH);
        delayMicroseconds(9);
        digitalWrite(IR_SND_PIN, LOW);
        delayMicroseconds(9);

        // so 26 microseconds altogether
        microsecs -= 26;
    }

    sei();  // this turns them back on
}

void ir_ctrl::send(String &code) {
    // parse input string
    uint8_t len = 0;
    uint8_t subidx = 0;
    int p = 0;
    int f = 0;
    while ((f = code.indexOf(',', p)) > -1) {
        m_buffer[len][subidx++] = code.substring(p, f).toInt();
        p = f + 1;
        if (subidx > 1) {
            subidx = 0;
            len++;
        }
    }
    // send code
    uint8_t repeat = 3;
    while (repeat-- > 0) {
        subidx = 0;
        for (uint8_t i = 0; i < len; i++) {
            int32_t t = m_buffer[i][0];
            t *= 10;
            pulse_ir(t);
            t = m_buffer[i][1];
            t *= 10;
            delayMicroseconds(t);
        }
        delay(10);
    }
}

bool ir_ctrl::receive(int8_t timeout) {
    m_currentpulse = 0;                                        // index for pulses we're storing
    int32_t timeout_micros = (int32_t) timeout * 1000 * 1000;  // timeout in micros
    while (true /*timeout_micros > 0*/) {
        uint16_t highpulse, lowpulse;  // temporary storage timing
        highpulse = lowpulse = 0;      // start out with no pulse length
        
        while (IR_PORT_REG & (1 << IR_RCV_PIN)) {
            // pin is still HIGH

            // count off another few microseconds
            highpulse++;
            delayMicroseconds(RESOLUTION);
            //timeout_micros -= RESOLUTION;

            // If the pulse is too long, we 'timed out' - either nothing
            // was received or the code is finished, so print what
            // we've grabbed so far, and then reset
            if ((highpulse >= MAX_PULSE) && (m_currentpulse != 0)) {
                return true;
            }
            // Global timeout
            //if (timeout_micros <= 0) {
            //    return false;
            //}
        }
        // we didn't time out so lets stash the reading
        m_buffer[m_currentpulse][0] = highpulse;

        // same as above
        while (!(IR_PORT_REG & _BV(IR_RCV_PIN))) {
            // pin is still LOW
            lowpulse++;
            delayMicroseconds(RESOLUTION);
            //timeout_micros -= RESOLUTION;
            if ((lowpulse >= MAX_PULSE) && (m_currentpulse != 0)) {
                return true;
            }
            //if (timeout_micros <= 0) {
            //    return false;
            //}
        }
        m_buffer[m_currentpulse][1] = lowpulse;

        // we read one high-low pulse successfully, continue!
        m_currentpulse++;
    }
    return false;
}

String ir_ctrl::code_string() const {
    String code;
    for (uint8_t i = 0; i < m_currentpulse - 1; i++) {
        code += m_buffer[i][1] * RESOLUTION / 10;
        code += ",";
        code += m_buffer[i + 1][0] * RESOLUTION / 10;
        code += ",";
    }
    code += m_buffer[m_currentpulse - 1][1] * RESOLUTION / 10;
    code += ",0";
    return code;
}
