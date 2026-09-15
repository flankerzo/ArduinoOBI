# Pylontech Dual Router




This is an early development project. Validate it on a bench before connecting
it to a live inverter/battery system.



This project is an ESPHome pattern for exposing one real Pylontech battery as two independent Pylontech UART interfaces for two inverter masters.

## Recommended example

Use the recommended example here:

- [pylontech-dual-proxy-example.yaml](pylontech-dual-proxy-example.yaml)

The custom component is under:

- [components/pylontech_dual_proxy](components/pylontech_dual_proxy)

## What it is for
- you have two pv inverters (in single mode), but only one battery, both inverters cannot be connected to same battery.
  Both are masters, with their identifiers, with this, you can connect 2pcs Pylontech compatible inverters to single battery.

## Hw used
ESP32 S3 16R8 module, 3x waveshare RS485 isolated to uart. 

## What this proxy does

- accepts only checksum-valid Pylontech ASCII frames (`~...\r`)
- queues requests from both inverter UARTs, sending one request at a time to the battery
- preserves the live Pylontech address and checksum exactly (the separate physical UART identifies each inverter)
- returns a matched reply only to the inverter that requested it
- forwards unmatched, valid battery frames to both inverter UARTs as unsolicited events
- logs raw Pylontech frames for diagnostics
- has independent HA switches for raw frames and decoded reply text;
  both are off by default and restore their saved setting after reboot
- exposes Battery, Inverter 1, and Inverter 2 connectivity binary sensors;
  each is online after a valid received frame and offline after its configured
  `link_timeout` (60 seconds in the example)
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

### Quiet battery diagnostics

[pylontech-dual-proxy-example.yaml](pylontech-dual-proxy-example.yaml) contains the supplied working
configuration with both diagnostic switches connected. It retains the GitHub
component source, UART pins, API encryption secret reference, and other settings.

In Home Assistant, turn off **Publish Battery Raw Frames** and **Publish Battery
Decoded Replies** to stop new updates to **Battery Last Frame** and **Battery Last
Decoded Reply**. Both default to off on first use and remember subsequent changes.
Existing text/history may remain visible in HA; turning these off does not erase it.
Normal battery sensors, UART forwarding, and console logging continue. Turn a
switch back on to resume its text updates as eligible battery replies arrive.

**Publish Inverter 1 Last Request** and **Publish Inverter 2 Last Request**
independently control new updates to each inverter's Last Request text sensor.
They also default to off and restore their saved setting after reboot. Each
inverter instance uses its own `publish_raw_frames` switch; routing and logging
continue regardless of the switch state. Configurations without this optional
switch retain their previous publishing behavior.

The inverter switches require the updated component in this repository. When
using the GitHub source, publish the component change there before building,
or use a local external component source pointing to `./components`.

This project is intentionally designed around raw Pylontech pass-through only.
The router works locally without a Home Assistant/API connection; the example
disables API-disconnect reboots so RS485 routing continues while HA is down.
The observed Pylontech `61` reply provides aggregate min/max cell values, not
individual readings for every battery cell.
It does not inject its own polling requests. This means no additional traffic
is added to the battery bus, while future inverters can still use any Pylontech
command that the battery implements.

## Safety

This is a custom electrical/protocol project. High-current battery and inverter wiring can be dangerous. Validate the wiring, pinout, RS485 polarity, and protections before making it live.
