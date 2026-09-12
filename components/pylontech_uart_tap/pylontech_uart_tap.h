#pragma once

#include <string>

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace pylontech_uart_tap {

// A diagnostic bridge only. It copies every received byte unchanged to its
// peer UART. Pylontech-looking frames are additionally logged after CR.
class PylontechUartTap : public Component, public uart::UARTDevice {
 public:
  void loop() override;
  void dump_config() override;

  void set_peer(PylontechUartTap *peer) { this->peer_ = peer; }
  void set_tap_name(const std::string &name) { this->tap_name_ = name; }
  void set_last_frame_sensor(text_sensor::TextSensor *sensor) { this->last_frame_sensor_ = sensor; }

 protected:
  void capture_for_log_(uint8_t byte);

  PylontechUartTap *peer_{nullptr};
  std::string tap_name_{"uart"};
  std::string frame_buffer_;
  text_sensor::TextSensor *last_frame_sensor_{nullptr};
};

}  // namespace pylontech_uart_tap
}  // namespace esphome
