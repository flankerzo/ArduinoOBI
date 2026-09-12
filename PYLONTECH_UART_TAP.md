# Pylontech UART tap

Use `pylontech-uart-tap-example.yaml` to capture a known-good, direct
inverter-to-BMS Pylontech V3.5 session. It is a transparent two-UART bridge:
every byte received on either side is immediately transmitted unchanged to the
other side.

The debug log labels complete ASCII Pylontech frames as either `inverter ->
battery` or `battery -> inverter`, and gives each direction its own increasing
frame number (for example, `inverter -> battery frame #17`). The number exists
only in the ESPHome log; it is never added to the wire message. The tap does
not use JK Modbus, modify addresses, validate/recalculate checksums, create
responses, or emulate a battery.

Connect only one inverter for this capture. Keep the existing direct inverter
to BMS session otherwise unchanged, then place the ESP32 bridge inline using
two isolated RS485-to-UART modules. Confirm RS485 A/B polarity on each module
before powering the system.
