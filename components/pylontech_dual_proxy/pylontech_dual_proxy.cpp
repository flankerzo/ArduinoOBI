#include "pylontech_dual_proxy.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

#include "esphome/core/log.h"

namespace esphome {
namespace pylontech_dual_proxy {

static const char *const TAG = "pylontech_dual_proxy";
std::vector<PylontechDualProxy *> PylontechDualProxy::active_instances_;
std::deque<PylontechDualProxy::PendingRequest> PylontechDualProxy::pending_requests_;
SharedBatterySnapshot PylontechDualProxy::shared_snapshot_{};

void PylontechDualProxy::setup() {
  register_instance_(this);
  if (online_sensor_ != nullptr) online_sensor_->publish_state(false);
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
  update_link_status_();
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

static uint16_t hex_u16_(const std::string &value, size_t offset) {
  if (offset + 4 > value.size()) return 0;
  unsigned int result = 0;
  sscanf(value.substr(offset, 4).c_str(), "%x", &result);
  return static_cast<uint16_t>(result);
}

static int16_t hex_i16_(const std::string &value, size_t offset) {
  return static_cast<int16_t>(hex_u16_(value, offset));
}

static uint8_t hex_u8_(const std::string &value, size_t offset) {
  if (offset + 2 > value.size()) return 0;
  unsigned int result = 0;
  sscanf(value.substr(offset, 2).c_str(), "%x", &result);
  return static_cast<uint8_t>(result);
}

static float decikelvin_to_celsius_(uint16_t value) { return (static_cast<float>(value) - 2731.0f) / 10.0f; }

static std::string ascii_from_hex_(const std::string &value, size_t offset, size_t byte_count) {
  std::string result;
  for (size_t i = 0; i < byte_count && offset + 2 <= value.size(); i++, offset += 2) {
    const char c = static_cast<char>(hex_u8_(value, offset));
    result += std::isprint(static_cast<unsigned char>(c)) ? c : '.';
  }
  return result;
}

number::Number *PylontechDualProxy::interval_number_for_cid_(const std::string &request_cid2) const {
  if (request_cid2 == "61") return gsad_update_interval_number_;
  if (request_cid2 == "63") return gscd_limits_update_interval_number_;
  if (request_cid2 == "42") return module_analog_update_interval_number_;
  if (request_cid2 == "47") return system_parameters_update_interval_number_;
  if (request_cid2 == "4F") return protocol_version_update_interval_number_;
  if (request_cid2 == "51") return manufacturer_info_update_interval_number_;
  if (request_cid2 == "92") return charge_management_update_interval_number_;
  if (request_cid2 == "93") return module_serial_update_interval_number_;
  if (request_cid2 == "96") return firmware_info_update_interval_number_;
  return nullptr;
}

bool PylontechDualProxy::should_publish_to_ha_(const std::string &request_cid2) {
  // Alarm data is safety-relevant: never rate-limit CID44 (module alarm) or
  // CID62 (system alarm). This never changes the UART/proxy timing.
  if (request_cid2 == "44" || request_cid2 == "62") return true;
  number::Number *interval = interval_number_for_cid_(request_cid2);
  // Per-command controls use -1 s as "inherit the global control".
  if (interval != nullptr && interval->state < 0.0f) interval = nullptr;
  if (interval == nullptr) interval = ha_update_interval_number_;
  const float seconds = interval == nullptr ? 0.0f : interval->state;
  if (seconds <= 0.0f) return true;
  const uint32_t now = millis();
  const uint32_t interval_ms = static_cast<uint32_t>(seconds * 1000.0f);
  const uint32_t last = last_ha_publish_ms_[request_cid2];
  if (last != 0 && now - last < interval_ms) return false;
  last_ha_publish_ms_[request_cid2] = now;
  return true;
}

void PylontechDualProxy::mark_link_active_() {
  last_link_activity_ms_ = millis();
  if (!link_online_) {
    link_online_ = true;
    if (online_sensor_ != nullptr) online_sensor_->publish_state(true);
  }
}

void PylontechDualProxy::update_link_status_() {
  if (!link_online_ || millis() - last_link_activity_ms_ < link_timeout_ms_) return;
  link_online_ = false;
  if (online_sensor_ != nullptr) online_sensor_->publish_state(false);
}

void PylontechDualProxy::publish_decoded_response_(const std::string &request_cid2, const std::string &info) {
  if (last_battery_snapshot_sensor_ == nullptr ||
      (publish_decoded_replies_switch_ != nullptr && !publish_decoded_replies_switch_->state))
    return;

  // The request CID2 identifies a normal reply (whose CID2 is always 00).
  // This diagnostic text deliberately does not alter a routed byte.
  std::ostringstream decoded;
  decoded.setf(std::ios::fixed);
  decoded.precision(3);
  if (request_cid2 == "42" && info.size() >= 4) {
    const uint8_t module = hex_u8_(info, 0);
    const uint8_t cells = hex_u8_(info, 2);
    size_t offset = 4 + static_cast<size_t>(cells) * 4;
    if (cells > 16 || offset + 2 > info.size()) return;
    const uint8_t temperatures = hex_u8_(info, offset);
    offset += 2;
    if (temperatures > 16 || offset + static_cast<size_t>(temperatures) * 4 + 12 > info.size()) return;
    decoded << "CID42 module=" << static_cast<unsigned>(module) << " cells=" << static_cast<unsigned>(cells)
            << " temperatures=" << static_cast<unsigned>(temperatures);
    if (cells > 0) {
      decoded << " cell1=" << static_cast<float>(hex_u16_(info, 4)) / 1000.0f << "V";
    }
    offset += static_cast<size_t>(temperatures) * 4;
    const float current = static_cast<float>(hex_i16_(info, offset)) / 100.0f;
    const float voltage = static_cast<float>(hex_u16_(info, offset + 4)) / 1000.0f;
    const float remaining_ah = static_cast<float>(hex_u16_(info, offset + 8)) / 1000.0f;
    decoded << " pack=" << voltage << "V current=" << current << "A remaining=" << remaining_ah << "Ah";
  } else if (request_cid2 == "44" && info.size() >= 4) {
    const uint8_t module = hex_u8_(info, 0);
    const uint8_t cells = hex_u8_(info, 2);
    size_t offset = 4 + cells * 2;
    if (cells > 16 || offset + 2 > info.size()) return;
    const uint8_t temperatures = hex_u8_(info, offset);
    offset += 2 + temperatures * 2;
    if (temperatures > 16 || offset + 16 > info.size()) return;
    decoded << "CID44 module=" << static_cast<unsigned>(module) << " cells=" << static_cast<unsigned>(cells)
            << " temperatures=" << static_cast<unsigned>(temperatures) << " warnings="
            << info.substr(offset, 6) << " status=" << info.substr(offset + 6, 10);
  } else if (request_cid2 == "47" && (info.size() == 48 || info.size() == 50)) {
    // Older Pylontech replies contain DATAI only (48 ASCII chars); some
    // variants prefix it with a one-byte INFOFLAG (50 ASCII chars).
    const size_t offset = info.size() == 50 ? 2 : 0;
    decoded << "CID47 cell-high=" << static_cast<float>(hex_u16_(info, offset)) / 1000.0f << "V"
            << " cell-low=" << static_cast<float>(hex_u16_(info, offset + 4)) / 1000.0f << "V"
            << " charge-temp=" << decikelvin_to_celsius_(hex_u16_(info, offset + 12)) << "C.."
            << decikelvin_to_celsius_(hex_u16_(info, offset + 16)) << "C"
            << " charge-limit=" << static_cast<float>(hex_i16_(info, offset + 20)) / 100.0f << "A"
            << " module-high=" << static_cast<float>(hex_u16_(info, offset + 24)) / 1000.0f << "V"
            << " discharge-limit=" << static_cast<float>(hex_i16_(info, offset + 44)) / 100.0f << "A";
  } else if (request_cid2 == "92" && info.size() == 20) {
    const uint8_t status = hex_u8_(info, 18);
    decoded << "CID92 module=" << static_cast<unsigned>(hex_u8_(info, 0))
            << " charge-max=" << static_cast<float>(hex_u16_(info, 2)) / 1000.0f << "V"
            << " discharge-min=" << static_cast<float>(hex_u16_(info, 6)) / 1000.0f << "V"
            << " charge-max=" << static_cast<float>(hex_i16_(info, 10)) / 10.0f << "A"
            << " discharge-max=" << static_cast<float>(hex_i16_(info, 14)) / 10.0f << "A"
            << " status=0x" << info.substr(18, 2)
            << " charge=" << ((status & 0x80) ? "enabled" : "stop")
            << " discharge=" << ((status & 0x40) ? "enabled" : "stop");
  } else if (request_cid2 == "61" && info.size() == 98) {
    decoded << "CID61 pack=" << static_cast<float>(hex_u16_(info, 0)) / 1000.0f << "V"
            << " current=" << static_cast<float>(hex_i16_(info, 4)) / 100.0f << "A"
            << " soc=" << static_cast<unsigned>(hex_u8_(info, 8)) << "%"
            << " cycles=" << hex_u16_(info, 10)
            << " soh=" << static_cast<unsigned>(hex_u8_(info, 18)) << "%";
  } else if (request_cid2 == "63" && info.size() == 18) {
    decoded << "CID63 charge-max=" << static_cast<float>(hex_u16_(info, 0)) / 1000.0f << "V"
            << " discharge-min=" << static_cast<float>(hex_u16_(info, 4)) / 1000.0f << "V"
            << " charge-max=" << static_cast<float>(hex_u16_(info, 8)) / 10.0f << "A"
            << " discharge-max=" << static_cast<float>(hex_u16_(info, 12)) / 10.0f << "A";
  } else if (request_cid2 == "4F") {
    decoded << "CID4F protocol-version=" << ascii_from_hex_(info, 0, info.size() / 2);
  } else if (request_cid2 == "51" && info.size() >= 64) {
    decoded << "CID51 battery=" << ascii_from_hex_(info, 0, 10)
            << " firmware=" << ascii_from_hex_(info, 20, 2)
            << " manufacturer=" << ascii_from_hex_(info, 24, 20);
  } else if (request_cid2 == "93" && info.size() >= 34) {
    decoded << "CID93 module=" << static_cast<unsigned>(hex_u8_(info, 0))
            << " serial=" << ascii_from_hex_(info, 2, 16);
  } else if (request_cid2 == "62" || request_cid2 == "96") {
    decoded << "CID" << request_cid2 << " raw-info=" << info;
  } else {
    // A V3.5 extension or an implementation-specific variant: retain it in
    // HA with its request CID rather than making up a field layout.
    decoded << "CID" << request_cid2 << " raw-info=" << info;
  }
  last_battery_snapshot_sensor_->publish_state(decoded.str());
}

bool PylontechDualProxy::update_snapshot_from_battery_frame_(const std::string &frame,
                                                              const std::string &request_cid2) {
  // A normal reply has CID2=00. LENID's low 12 bits are the ASCII INFO length;
  // its high nibble is LCHKSUM. Do not interpret LENID as a plain length.
  if (!is_valid_frame_(frame) || frame_cid2_(frame) != "00" || frame.size() < 18) return false;
  unsigned int lenid = 0;
  sscanf(frame.substr(9, 4).c_str(), "%x", &lenid);
  const size_t info_len = lenid & 0x0fff;
  if (info_len != frame.size() - 18) return false;
  const std::string info = frame.substr(13, info_len);
  if (!should_publish_to_ha_(request_cid2)) return true;
  publish_decoded_response_(request_cid2, info);

  if (request_cid2 == "61" && info.size() == 98) {
    const float voltage = static_cast<float>(hex_u16_(info, 0)) / 1000.0f;
    const float current = static_cast<float>(hex_i16_(info, 4)) / 100.0f;
    const float soc = static_cast<float>(hex_u8_(info, 8));
    const float cycles = static_cast<float>(hex_u16_(info, 10));
    const float soh = static_cast<float>(hex_u8_(info, 18));
    const float max_cell_voltage = static_cast<float>(hex_u16_(info, 22)) / 1000.0f;
    const float min_cell_voltage = static_cast<float>(hex_u16_(info, 30)) / 1000.0f;
    const float cell_temperature = decikelvin_to_celsius_(hex_u16_(info, 38));
    const float max_temperature = decikelvin_to_celsius_(hex_u16_(info, 42));
    const float min_temperature = decikelvin_to_celsius_(hex_u16_(info, 50));
    const float mosfet_temperature = decikelvin_to_celsius_(hex_u16_(info, 58));
    const float max_mosfet_temperature = decikelvin_to_celsius_(hex_u16_(info, 62));
    const float min_mosfet_temperature = decikelvin_to_celsius_(hex_u16_(info, 70));
    const float bms_temperature = decikelvin_to_celsius_(hex_u16_(info, 78));
    const float max_bms_temperature = decikelvin_to_celsius_(hex_u16_(info, 82));
    const float min_bms_temperature = decikelvin_to_celsius_(hex_u16_(info, 90));

    if (soc_sensor_ != nullptr) soc_sensor_->publish_state(soc);
    if (voltage_sensor_ != nullptr) voltage_sensor_->publish_state(voltage);
    if (current_sensor_ != nullptr) current_sensor_->publish_state(current);
    if (temperature_sensor_ != nullptr) temperature_sensor_->publish_state(cell_temperature);
    if (soh_sensor_ != nullptr) soh_sensor_->publish_state(soh);
    if (cycle_count_sensor_ != nullptr) cycle_count_sensor_->publish_state(cycles);
    // The JK Pylontech implementation uses 0000 for several unsupported
    // extrema. Do not publish physically impossible zero-voltage/-273.1 C.
    if (max_cell_voltage_sensor_ != nullptr && hex_u16_(info, 22) != 0)
      max_cell_voltage_sensor_->publish_state(max_cell_voltage);
    if (min_cell_voltage_sensor_ != nullptr && hex_u16_(info, 30) != 0)
      min_cell_voltage_sensor_->publish_state(min_cell_voltage);
    if (max_temperature_sensor_ != nullptr && hex_u16_(info, 42) != 0)
      max_temperature_sensor_->publish_state(max_temperature);
    if (min_temperature_sensor_ != nullptr && hex_u16_(info, 50) != 0)
      min_temperature_sensor_->publish_state(min_temperature);
    if (mosfet_temperature_sensor_ != nullptr && hex_u16_(info, 58) != 0)
      mosfet_temperature_sensor_->publish_state(mosfet_temperature);
    if (max_mosfet_temperature_sensor_ != nullptr && hex_u16_(info, 62) != 0)
      max_mosfet_temperature_sensor_->publish_state(max_mosfet_temperature);
    if (min_mosfet_temperature_sensor_ != nullptr && hex_u16_(info, 70) != 0)
      min_mosfet_temperature_sensor_->publish_state(min_mosfet_temperature);
    if (bms_temperature_sensor_ != nullptr && hex_u16_(info, 78) != 0)
      bms_temperature_sensor_->publish_state(bms_temperature);
    if (max_bms_temperature_sensor_ != nullptr && hex_u16_(info, 82) != 0)
      max_bms_temperature_sensor_->publish_state(max_bms_temperature);
    if (min_bms_temperature_sensor_ != nullptr && hex_u16_(info, 90) != 0)
      min_bms_temperature_sensor_->publish_state(min_bms_temperature);
    if (battery_charging_sensor_ != nullptr) battery_charging_sensor_->publish_state(current > 0.0f);
    if (battery_discharging_sensor_ != nullptr) battery_discharging_sensor_->publish_state(current < 0.0f);

    shared_snapshot_.valid = true;
    shared_snapshot_.updated_ms = millis();
    shared_snapshot_.voltage_mv = hex_u16_(info, 0);
    shared_snapshot_.current_ca = hex_i16_(info, 4);
    shared_snapshot_.soc_percent = hex_u8_(info, 8);
    shared_snapshot_.max_cell_v_mv = hex_u16_(info, 22);
    shared_snapshot_.min_cell_v_mv = hex_u16_(info, 30);
    return true;
  }

  if (request_cid2 == "63" && info.size() == 18) {
    const float max_charge_voltage = static_cast<float>(hex_u16_(info, 0)) / 1000.0f;
    const float min_discharge_voltage = static_cast<float>(hex_u16_(info, 4)) / 1000.0f;
    const float max_charge_current = static_cast<float>(hex_u16_(info, 8)) / 10.0f;
    const float max_discharge_current = static_cast<float>(hex_u16_(info, 12)) / 10.0f;
    if (max_voltage_sensor_ != nullptr) max_voltage_sensor_->publish_state(max_charge_voltage);
    if (min_voltage_sensor_ != nullptr) min_voltage_sensor_->publish_state(min_discharge_voltage);
    if (max_charge_current_sensor_ != nullptr) max_charge_current_sensor_->publish_state(max_charge_current);
    if (max_discharge_current_sensor_ != nullptr) max_discharge_current_sensor_->publish_state(max_discharge_current);
    shared_snapshot_.max_charge_v_mv = hex_u16_(info, 0);
    shared_snapshot_.min_discharge_v_mv = hex_u16_(info, 4);
    shared_snapshot_.max_charge_i_da = hex_u16_(info, 8);
    shared_snapshot_.max_discharge_i_da = hex_u16_(info, 12);
    return true;
  }

  return false;
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
        mark_link_active_();
        if (last_inverter_request_sensor_ != nullptr &&
            (publish_raw_frames_switch_ == nullptr || publish_raw_frames_switch_->state))
          last_inverter_request_sensor_->publish_state(request);
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
        mark_link_active_();
        if (last_battery_frame_sensor_ != nullptr &&
            (publish_raw_frames_switch_ == nullptr || publish_raw_frames_switch_->state))
          last_battery_frame_sensor_->publish_state(response);
        last_battery_response_ = response;
        log_raw_frame_("battery->router", response);
        // The working inverter/emulator capture shows requests with CID2 61/63
        // and their replies with CID2 00. A CID2 00 frame is therefore the next
        // reply for the single outstanding transaction, not an event.
        if (!pending_requests_.empty() && pending_requests_.front().sent_ms != 0 &&
            frame_cid2_(response) == "00") {
          const PendingRequest pending = pending_requests_.front();
          pending_requests_.pop_front();
          this->update_snapshot_from_battery_frame_(response, pending.cid2);
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
