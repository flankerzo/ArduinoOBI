# Pylontech Dual Proxy




This is an early development project. Validate it on a bench before connecting
it to a live inverter/battery system.



This project is an ESPHome pattern for exposing one real Pylontech battery as two independent Pylontech UART interfaces for two inverter masters.

## Recommended example

Use the recommended example here:

- [pylontech-dual-proxy-example.yaml](pylontech-dual-proxy-example.yaml)

The custom component is under:

- [components/pylontech_dual_proxy](components/pylontech_dual_proxy)

## What this proxy does

- accepts only checksum-valid Pylontech ASCII frames (`~...\r`)
- queues requests from both inverter UARTs, sending one request at a time to the battery
- preserves the live Pylontech address and checksum exactly (the separate physical UART identifies each inverter)
- returns a matched reply only to the inverter that requested it
- forwards unmatched, valid battery frames to both inverter UARTs as unsolicited events
- logs raw Pylontech frames for diagnostics
- read-only publishes Pylontech V3.5 `61`/`63` replies to Home Assistant:
  pack voltage/current/SOC, SOH, cycles, min/max cell voltage, temperatures,
  and charge/discharge limits
- decodes documented Pylontech command replies (`42`, `44`, `47`, `4F`, `51`,
  `92`, `93`, and `96`) into the **Battery Last Decoded Reply** diagnostic
  text sensor when an inverter requests one; variant/unknown replies are kept
  as their CID-tagged raw INFO rather than guessed
- includes live Home Assistant update-rate controls in seconds: a global
  default plus per-command overrides. `0` publishes every received response;
  `-1` on an override inherits the global setting. Pylontech alarm replies
  (`44` and `62`) always publish immediately.

## Recommended usage

1. Copy the example YAML into your ESPHome config.
2. Update the UART pins and Wi-Fi credentials.
3. Build and upload through Home Assistant / ESPHome Dashboard.

## Notes

This project is intentionally designed around raw Pylontech pass-through only.
It does not speak JK Modbus or map JK Modbus registers.
The observed Pylontech `61` reply provides aggregate min/max cell values, not
individual readings for every battery cell.
It does not inject its own polling requests. This means no additional traffic
is added to the battery bus, while future inverters can still use any Pylontech
command that the battery implements.

## Safety

This is a custom electrical/protocol project. High-current battery and inverter wiring can be dangerous. Validate the wiring, pinout, RS485 polarity, and protections before making it live.
