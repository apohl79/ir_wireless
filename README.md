# ir_wireless

This is a simple IR-wireless bridge. You need the following components:

- Arduino µC
- ESP8266 wireless module
- IR diode
- IR receiver
- LED
- some resistors (220 Ohm, 330 Ohm)

What it does?
-------------

1) Connects to your wireless network

2) Connects to a control server in your network on port 98 (this is a bridge server forwarding requests from a client to the IR controller conntection, see the scripts folder to understand how to use this)

3) You can send two commands

3.1) RCV - activates IR receiver to decode an IR signal and sends back the code

3.2) SND code - Sends an IR code sequence via the IR diode
