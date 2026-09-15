# Project configuration rules

- Never add `update_interval` to ESPHome template `binary_sensor` entries,
  including `update_interval: never` or commented examples of it. These binary
  sensors are published by the router component.
- This restriction is specific to binary sensors; preserve valid
  `update_interval: never` settings on template `sensor` and `text_sensor` entries.
