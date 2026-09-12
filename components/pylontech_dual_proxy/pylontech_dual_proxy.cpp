#include "pylontech_dual_proxy.h"

#include <algorithm>
#include <cctype>
#include <cstdio>

#include "esphome/core/log.h"

namespace esphome {
namespace pylontech_dual_proxy {

static const char *const TAG = "pylontech_dual_proxy";
std::vector<PylontechDualProxy *> PylontechDualProxy::active_instances_;
std::deque<PylontechDualProxy::PendingRequest> PylontechDualProxy::pending_requests_;
SharedBatterySnapshot PylontechDualProxy::shared_snapshot_{};

void PylontechDualProxy::setup() {
  register_instance_(this);
  ESP_LOGCONFIG(TAG, "Pylontech V3.5 queued router starting");
}

void PylontechDualProxy::dump_config() {
  ESP_LOGCONFIG(TAG, "Pylontech V3.5 queued router");
  ESP_LOGCONFIG(TAG, "  Port role: %s", this->is_battery_port_ ? "battery" : "inverter");
  ESP_LOGCONFIG(TAG, "  Reply timeout: %lu ms", static_cast<unsigned long>(this->response_timeout_ms_));
}

void PylontechDualProxy::loop() {
  if (this->is_battery_port_) {
    read_battery_frames_();
    expire_pending_request_();
    start_next_request_();
  } else {
    read_inverter_requests_();
  }
}

bool PylontechDualProxy::is_valid_frame_(const std::string &frame) {
  // Pylontech RS485: '~' + ASCII hexadecimal header/info + 4 ASCII hex checksum + CR.
  if (frame.size() < 14 || frame.front() != '~' || frame.back() != '\r') return false;
  for (size_t i = 1; i + 1 < frame.size(); i++) {
    if (!std::isxdigit(static_cast<unsigned char>(frame[i]))) return false;
  }
  const std::string payload = frame.substr(1, frame.size() - 6);
  return calculate_checksum_(payload) == frame.substr(frame.size() - 5, 4);
}

std::string PylontechDualProxy::frame_address_(const std::string &frame) {
  // ~ VV AA C1 C2 LLLL [INFO] CCCC CR
  return frame.size() >= 5 ? frame.substr(3, 2) : "";
}

std::string PylontechDualProxy::frame_cid2_(const std::string &frame) {
  return frame.size() >= 9 ? frame.substr(7, 2) : "";
}

void PylontechDualProxy::read_inverter_requests_() {
  while (available()) {
    uint8_t byte;
    if (!read_byte(&byte)) continue;
    const char c = static_cast<char>(byte);
    if (c == '~') {
      rx_buffer_.assign(1, c);
    } else if (!rx_buffer_.empty()) {
      rx_buffer_ += c;
      if (rx_buffer_.size() > 1024) {
        ESP_LOGW(TAG, "Discarding oversized inverter frame.");
        rx_buffer_.clear();
        continue;
      }
      if (c == '\r') {
        const std::string request = rx_buffer_;
        rx_buffer_.clear();
        if (!is_valid_frame_(request)) {
          ESP_LOGW(TAG, "Discarding invalid inverter frame: %s", request.c_str());
          continue;
        }
        if (last_inverter_request_sensor_ != nullptr) last_inverter_request_sensor_->publish_state(request);
        log_raw_frame_("inverter->router", request);
        for (auto *instance : active_instances_) {
          if (instance != nullptr && instance->is_battery_port_) {
            instance->enqueue_request_(request, this);
            break;
          }
        }
      }
    }
  }
}

void PylontechDualProxy::enqueue_request_(const std::string &request, PylontechDualProxy *requester) {
  if (requester == nullptr) return;
  if (pending_requests_.size() >= 8) {
    ESP_LOGW(TAG, "Request queue is full; dropping inverter request from address %s.", frame_address_(request).c_str());
    return;
  }
  PendingRequest pending;
  pending.requester = requester;
  pending.request = request;
  pending.cid2 = frame_cid2_(request);
  pending_requests_.push_back(pending);
  start_next_request_();
}

void PylontechDualProxy::start_next_request_() {
  if (!is_battery_port_ || pending_requests_.empty() || pending_requests_.front().sent_ms != 0) return;
  PendingRequest &pending = pending_requests_.front();
  if (pending.requester == nullptr) {
    pending_requests_.pop_front();
    return;
  }
  const std::string &to_battery = pending.request;
  pending.sent_ms = millis();
  log_raw_frame_("router->battery", to_battery);
  write_str(to_battery.c_str());
}

void PylontechDualProxy::expire_pending_request_() {
  if (pending_requests_.empty() || pending_requests_.front().sent_ms == 0 ||
      millis() - pending_requests_.front().sent_ms <= response_timeout_ms_) return;
  ESP_LOGW(TAG, "Battery response timed out for CID2=%s; moving to next request.",
           pending_requests_.front().cid2.c_str());
  pending_requests_.pop_front();
}

void PylontechDualProxy::read_battery_frames_() {
  while (available()) {
    uint8_t byte;
    if (!read_byte(&byte)) continue;
    const char c = static_cast<char>(byte);
    if (c == '~') {
      rx_buffer_.assign(1, c);
    } else if (!rx_buffer_.empty()) {
      rx_buffer_ += c;
      if (rx_buffer_.size() > 1024) {
        ESP_LOGW(TAG, "Discarding oversized battery frame.");
        rx_buffer_.clear();
        continue;
      }
      if (c == '\r') {
        const std::string response = rx_buffer_;
        rx_buffer_.clear();
        if (!is_valid_frame_(response)) {
          ESP_LOGW(TAG, "Discarding invalid battery frame: %s", response.c_str());
          continue;
        }
        if (last_battery_frame_sensor_ != nullptr) last_battery_frame_sensor_->publish_state(response);
        last_battery_response_ = response;
        log_raw_frame_("battery->router", response);
        // The working inverter/emulator capture shows requests with CID2 61/63
        // and their replies with CID2 00. A CID2 00 frame is therefore the next
        // reply for the single outstanding transaction, not an event.
        if (!pending_requests_.empty() && pending_requests_.front().sent_ms != 0 &&
            frame_cid2_(response) == "00") {
          const PendingRequest pending = pending_requests_.front();
          pending_requests_.pop_front();
          if (pending.requester != nullptr) {
            pending.requester->log_raw_frame_("router->inverter reply", response);
            pending.requester->write_str(response.c_str());
          }
        } else {
          // An unmatched valid frame is treated as an unsolicited battery event/alarm.
          fan_out_battery_event_(response);
        }
      }
    }
  }
}

void PylontechDualProxy::fan_out_battery_event_(const std::string &frame) {
  for (auto *instance : active_instances_) {
    if (instance == nullptr || instance->is_battery_port_) continue;
    instance->log_raw_frame_("router->inverter unsolicited", frame);
    instance->write_str(frame.c_str());
    if (instance->last_battery_event_sensor_ != nullptr) instance->last_battery_event_sensor_->publish_state(frame);
  }
}

void PylontechDualProxy::register_instance_(PylontechDualProxy *proxy) {
  if (proxy != nullptr && std::find(active_instances_.begin(), active_instances_.end(), proxy) == active_instances_.end())
    active_instances_.push_back(proxy);
}

void PylontechDualProxy::unregister_instance_(PylontechDualProxy *proxy) {
  active_instances_.erase(std::remove(active_instances_.begin(), active_instances_.end(), proxy), active_instances_.end());
}

void PylontechDualProxy::broadcast_to_all_inverters_(const std::string &frame, PylontechDualProxy *sender) {
  if (sender != nullptr) sender->fan_out_battery_event_(frame);
}

void PylontechDualProxy::log_raw_frame_(const std::string &label, const std::string &frame) {
  ESP_LOGD(TAG, "%s: %s", label.c_str(), frame.c_str());
}

bool PylontechDualProxy::is_battery_event_frame_(const std::string &frame) const { return is_valid_frame_(frame); }

std::string PylontechDualProxy::calculate_checksum_(const std::string &payload) {
  uint16_t sum = 0;
  for (char c : payload) sum = static_cast<uint16_t>(sum + static_cast<uint8_t>(c));
  char checksum[5];
  snprintf(checksum, sizeof(checksum), "%04X", static_cast<uint16_t>(~sum + 1));
  return checksum;
}

std::string PylontechDualProxy::calculate_length_field_(size_t info_len) {
  char length[5];
  snprintf(length, sizeof(length), "%04X", static_cast<unsigned int>(info_len));
  return length;
}

}  // namespace pylontech_dual_proxy
}  // namespace esphome
