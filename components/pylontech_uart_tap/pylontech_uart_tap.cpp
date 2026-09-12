#include "pylontech_uart_tap.h"

#include "esphome/core/log.h"

namespace esphome {
namespace pylontech_uart_tap {

static const char *const TAG = "pylontech_uart_tap";

void PylontechUartTap::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech UART tap: %s", this->tap_name_.c_str());
}

void PylontechUartTap::loop() {
  while (this->available()) {
    uint8_t byte;
    if (!this->read_byte(&byte)) continue;

    // Transparent bridge: no filtering, parsing, addressing, checksum work,
    // or timing changes beyond receiving and immediately transmitting a byte.
    if (this->peer_ != nullptr) this->peer_->write_byte(byte);
    this->capture_for_log_(byte);
  }
}

void PylontechUartTap::capture_for_log_(uint8_t byte) {
  const char c = static_cast<char>(byte);
  if (c == '~') {
    this->frame_buffer_.assign(1, c);
    return;
  }
  if (this->frame_buffer_.empty()) return;

  this->frame_buffer_ += c;
  if (this->frame_buffer_.size() > 256) {
    ESP_LOGW(TAG, "%s: discarding oversized/incomplete frame", this->tap_name_.c_str());
    this->frame_buffer_.clear();
    return;
  }
  if (c != '\r') return;

  ESP_LOGD(TAG, "%s: %s", this->tap_name_.c_str(), this->frame_buffer_.c_str());
  if (this->last_frame_sensor_ != nullptr) this->last_frame_sensor_->publish_state(this->frame_buffer_);
  this->frame_buffer_.clear();
}

}  // namespace pylontech_uart_tap
}  // namespace esphome
