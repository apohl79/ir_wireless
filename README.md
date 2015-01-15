# ir_wireless

This is a simple IR-wireless bridge. You need

- Arduino µC
- ESP8266 wireless module
- IR diode
- IR receiver
- LED
- some resistors (220 Ohm, 330 Ohm)

What does it?
-------------

1) Connects to you wireless network

2) Opens a socket server on port 99

3) You can send two commands

3.1) RCV - activates IR receiver to decode an IR signal and sends back the code

3.2) SND code - Sends an IR code sequence via the IR diode
