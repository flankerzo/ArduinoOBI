#pragma once

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/number/number.h"
#include "esphome/components/switch/switch.h"
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

namespace esphome {
namespace pylontech_dual_proxy {

struct SharedBatterySnapshot {
  bool valid{false};
  uint32_t updated_ms{0};
  uint16_t soc_percent{0};
  uint16_t voltage_mv{0};
  int16_t current_ca{0};
  uint16_t temp_decik{0};
  uint16_t max_charge_v_mv{0};
  uint16_t min_discharge_v_mv{0};
  uint16_t max_charge_i_da{0};
  uint16_t max_discharge_i_da{0};
  uint16_t max_cell_v_mv{0};
  uint16_t min_cell_v_mv{0};
  uint16_t max_temp_decik{0};
  uint16_t min_temp_decik{0};
  uint16_t mosfet_temp_decik{0};
  uint16_t max_mosfet_temp_decik{0};
  uint16_t min_mosfet_temp_decik{0};
  uint16_t bms_temp_decik{0};
  uint16_t max_bms_temp_decik{0};
  uint16_t min_bms_temp_decik{0};
  uint16_t cycle_count{0};
  uint8_t soh_percent{100};
  bool force_charge_requested{false};
  uint8_t alarm_status_1{0};
  uint8_t alarm_status_2{0};
  uint8_t protection_status_1{0};
  uint8_t protection_status_2{0};
};

class PylontechDualProxy : public Component, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  void set_update_timeout(uint32_t timeout) { this->update_timeout_ms_ = timeout; }
  void set_cache_ttl(uint32_t ttl) { this->cache_ttl_ms_ = ttl; }
  void set_battery_port(bool battery_port) { this->is_battery_port_ = battery_port; }
  void set_response_timeout(uint32_t timeout) { this->response_timeout_ms_ = timeout; }
  void set_link_timeout(uint32_t timeout) { this->link_timeout_ms_ = timeout; }
  void set_publish_raw_frames_switch(switch_::Switch *value) { this->publish_raw_frames_switch_ = value; }
  void set_publish_decoded_replies_switch(switch_::Switch *value) { this->publish_decoded_replies_switch_ = value; }
  void set_ha_update_interval_number(number::Number *number) { this->ha_update_interval_number_ = number; }
  void set_gsad_update_interval_number(number::Number *number) { this->gsad_update_interval_number_ = number; }
  void set_gscd_limits_update_interval_number(number::Number *number) { this->gscd_limits_update_interval_number_ = number; }
  void set_module_analog_update_interval_number(number::Number *number) { this->module_analog_update_interval_number_ = number; }
  void set_system_parameters_update_interval_number(number::Number *number) { this->system_parameters_update_interval_number_ = number; }
  void set_protocol_version_update_interval_number(number::Number *number) { this->protocol_version_update_interval_number_ = number; }
  void set_manufacturer_info_update_interval_number(number::Number *number) { this->manufacturer_info_update_interval_number_ = number; }
  void set_charge_management_update_interval_number(number::Number *number) { this->charge_management_update_interval_number_ = number; }
  void set_module_serial_update_interval_number(number::Number *number) { this->module_serial_update_interval_number_ = number; }
  void set_firmware_info_update_interval_number(number::Number *number) { this->firmware_info_update_interval_number_ = number; }

  bool refresh_if_needed_();
  void ensure_cache_();
  void handle_unknown_command_(const std::string &cid);
  void handle_passthrough_system_command_(const std::string &request, const std::string &cid2);
  void handle_passthrough_to_battery_(const std::string &request);
  void handle_battery_response_passthrough_(const std::string &response);
  bool should_dedupe_request_(const std::string &request);
  std::string request_key_(const std::string &request) const;
  std::string rewrite_originator_(const std::string &frame, const std::string &new_originator) const;
  void log_raw_frame_(const std::string &label, const std::string &frame);
  bool is_battery_event_frame_(const std::string &frame) const;
  void fan_out_battery_event_(const std::string &frame);
  void log_battery_event_result_(const std::string &event, uint8_t ack_count, uint8_t inverter_count);
  static void register_instance_(PylontechDualProxy *proxy);
  static void unregister_instance_(PylontechDualProxy *proxy);
  static void broadcast_to_all_inverters_(const std::string &frame, PylontechDualProxy *sender);
  PylontechDualProxy *pending_inverter_requester_{nullptr};

  void set_soc_sensor(sensor::Sensor *sensor) { this->soc_sensor_ = sensor; }
  void set_voltage_sensor(sensor::Sensor *sensor) { this->voltage_sensor_ = sensor; }
  void set_current_sensor(sensor::Sensor *sensor) { this->current_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }
  void set_soh_sensor(sensor::Sensor *sensor) { this->soh_sensor_ = sensor; }
  void set_cycle_count_sensor(sensor::Sensor *sensor) { this->cycle_count_sensor_ = sensor; }
  void set_max_temperature_sensor(sensor::Sensor *sensor) { this->max_temperature_sensor_ = sensor; }
  void set_min_temperature_sensor(sensor::Sensor *sensor) { this->min_temperature_sensor_ = sensor; }
  void set_max_cell_voltage_sensor(sensor::Sensor *sensor) { this->max_cell_voltage_sensor_ = sensor; }
  void set_min_cell_voltage_sensor(sensor::Sensor *sensor) { this->min_cell_voltage_sensor_ = sensor; }
  void set_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->mosfet_temperature_sensor_ = sensor; }
  void set_max_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->max_mosfet_temperature_sensor_ = sensor; }
  void set_min_mosfet_temperature_sensor(sensor::Sensor *sensor) { this->min_mosfet_temperature_sensor_ = sensor; }
  void set_bms_temperature_sensor(sensor::Sensor *sensor) { this->bms_temperature_sensor_ = sensor; }
  void set_max_bms_temperature_sensor(sensor::Sensor *sensor) { this->max_bms_temperature_sensor_ = sensor; }
  void set_min_bms_temperature_sensor(sensor::Sensor *sensor) { this->min_bms_temperature_sensor_ = sensor; }

  void set_max_voltage_sensor(sensor::Sensor *sensor) { this->max_voltage_sensor_ = sensor; }
  void set_min_voltage_sensor(sensor::Sensor *sensor) { this->min_voltage_sensor_ = sensor; }
  void set_max_charge_current_sensor(sensor::Sensor *sensor) { this->max_charge_current_sensor_ = sensor; }
  void set_max_discharge_current_sensor(sensor::Sensor *sensor) { this->max_discharge_current_sensor_ = sensor; }

#ifdef USE_BINARY_SENSOR
  void set_requested_force_charge(binary_sensor::BinarySensor *sensor) { this->requested_force_charge_ = sensor; }
  void set_total_voltage_high_alarm(binary_sensor::BinarySensor *sensor) { this->total_voltage_high_alarm_ = sensor; }
  void set_total_voltage_low_alarm(binary_sensor::BinarySensor *sensor) { this->total_voltage_low_alarm_ = sensor; }
  void set_cell_voltage_high_alarm(binary_sensor::BinarySensor *sensor) { this->cell_voltage_high_alarm_ = sensor; }
  void set_cell_voltage_low_alarm(binary_sensor::BinarySensor *sensor) { this->cell_voltage_low_alarm_ = sensor; }
  void set_cell_temp_high_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_high_alarm_ = sensor; }
  void set_cell_temp_low_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_low_alarm_ = sensor; }
  void set_mosfet_temp_high_alarm(binary_sensor::BinarySensor *sensor) { this->mosfet_temp_high_alarm_ = sensor; }
  void set_cell_imbalance_alarm(binary_sensor::BinarySensor *sensor) { this->cell_imbalance_alarm_ = sensor; }
  void set_cell_temp_imbalance_alarm(binary_sensor::BinarySensor *sensor) { this->cell_temp_imbalance_alarm_ = sensor; }
  void set_charge_overcurrent_alarm(binary_sensor::BinarySensor *sensor) { this->charge_overcurrent_alarm_ = sensor; }
  void set_discharge_overcurrent_alarm(binary_sensor::BinarySensor *sensor) { this->discharge_overcurrent_alarm_ = sensor; }
  void set_module_overvoltage_protection(binary_sensor::BinarySensor *sensor) { this->module_overvoltage_protection_ = sensor; }
  void set_module_undervoltage_protection(binary_sensor::BinarySensor *sensor) { this->module_undervoltage_protection_ = sensor; }
  void set_cell_overvoltage_protection(binary_sensor::BinarySensor *sensor) { this->cell_overvoltage_protection_ = sensor; }
  void set_cell_undervoltage_protection(binary_sensor::BinarySensor *sensor) { this->cell_undervoltage_protection_ = sensor; }
  void set_cell_overtemp_protection(binary_sensor::BinarySensor *sensor) { this->cell_overtemp_protection_ = sensor; }
  void set_cell_undertemp_protection(binary_sensor::BinarySensor *sensor) { this->cell_undertemp_protection_ = sensor; }
  void set_mosfet_overtemp_protection(binary_sensor::BinarySensor *sensor) { this->mosfet_overtemp_protection_ = sensor; }
  void set_charge_overcurrent_protection(binary_sensor::BinarySensor *sensor) { this->charge_overcurrent_protection_ = sensor; }
  void set_discharge_overcurrent_protection(binary_sensor::BinarySensor *sensor) { this->discharge_overcurrent_protection_ = sensor; }
  void set_system_fault_protection(binary_sensor::BinarySensor *sensor) { this->system_fault_protection_ = sensor; }
#endif

  void set_last_battery_frame_sensor(text_sensor::TextSensor *sensor) { this->last_battery_frame_sensor_ = sensor; }
  void set_last_inverter_request_sensor(text_sensor::TextSensor *sensor) { this->last_inverter_request_sensor_ = sensor; }
  void set_last_battery_event_sensor(text_sensor::TextSensor *sensor) { this->last_battery_event_sensor_ = sensor; }
  void set_last_inverter_heartbeat_sensor(text_sensor::TextSensor *sensor) { this->last_inverter_heartbeat_sensor_ = sensor; }
  void set_last_battery_snapshot_sensor(text_sensor::TextSensor *sensor) { this->last_battery_snapshot_sensor_ = sensor; }
  void set_battery_charging_sensor(binary_sensor::BinarySensor *sensor) { this->battery_charging_sensor_ = sensor; }
  void set_battery_discharging_sensor(binary_sensor::BinarySensor *sensor) { this->battery_discharging_sensor_ = sensor; }
  void set_online_sensor(binary_sensor::BinarySensor *sensor) { this->online_sensor_ = sensor; }

 protected:
 public:
  struct PendingRequest {
    PylontechDualProxy *requester{nullptr};
    std::string request;
    std::string cid2;
    uint32_t sent_ms{0};
  };

 protected:
  void enqueue_request_(const std::string &request, PylontechDualProxy *requester);
  void start_next_request_();
  void expire_pending_request_();
  static bool is_valid_frame_(const std::string &frame);
  static std::string frame_address_(const std::string &frame);
  static std::string frame_cid2_(const std::string &frame);
  bool refresh_battery_snapshot_();
  bool update_snapshot_from_battery_frame_(const std::string &frame, const std::string &request_cid2);
  bool should_publish_to_ha_(const std::string &request_cid2);
  number::Number *interval_number_for_cid_(const std::string &request_cid2) const;
  void mark_link_active_();
  void update_link_status_();
  void publish_decoded_response_(const std::string &request_cid2, const std::string &info);
  void read_inverter_requests_();
  void read_battery_frames_();
  void route_request_(const std::string &request);
  void route_battery_response_to_inverter_(const std::string &response);
  void handle_command_60_(const std::string &request);
  void handle_command_61_(const std::string &request);
  void handle_command_62_(const std::string &request);
  void handle_command_63_(const std::string &request);
  void handle_command_64_(const std::string &request);
  void handle_command_65_(const std::string &request);
  void handle_command_66_(const std::string &request);
  void handle_command_67_(const std::string &request);
  void handle_command_68_(const std::string &request);
  void handle_command_69_(const std::string &request);
  void handle_command_6a_(const std::string &request);
  void handle_command_6b_(const std::string &request);
  void handle_command_6c_(const std::string &request);
  void handle_command_6d_(const std::string &request);
  void handle_command_6e_(const std::string &request);
  void handle_command_6f_(const std::string &request);

  static std::string calculate_checksum_(const std::string &frame_data);
  static std::string calculate_length_field_(size_t info_len);
  std::string build_response_frame_(const std::string &info_payload) const;

  sensor::Sensor *soc_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *soh_sensor_{nullptr};
  sensor::Sensor *cycle_count_sensor_{nullptr};
  sensor::Sensor *max_temperature_sensor_{nullptr};
  sensor::Sensor *min_temperature_sensor_{nullptr};
  sensor::Sensor *max_cell_voltage_sensor_{nullptr};
  sensor::Sensor *min_cell_voltage_sensor_{nullptr};
  sensor::Sensor *mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *max_mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *min_mosfet_temperature_sensor_{nullptr};
  sensor::Sensor *bms_temperature_sensor_{nullptr};
  sensor::Sensor *max_bms_temperature_sensor_{nullptr};
  sensor::Sensor *min_bms_temperature_sensor_{nullptr};

  sensor::Sensor *max_voltage_sensor_{nullptr};
  sensor::Sensor *min_voltage_sensor_{nullptr};
  sensor::Sensor *max_charge_current_sensor_{nullptr};
  sensor::Sensor *max_discharge_current_sensor_{nullptr};

#ifdef USE_BINARY_SENSOR
  binary_sensor::BinarySensor *requested_force_charge_{nullptr};
  binary_sensor::BinarySensor *total_voltage_high_alarm_{nullptr};
  binary_sensor::BinarySensor *total_voltage_low_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_voltage_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_voltage_low_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_low_alarm_{nullptr};
  binary_sensor::BinarySensor *mosfet_temp_high_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_imbalance_alarm_{nullptr};
  binary_sensor::BinarySensor *cell_temp_imbalance_alarm_{nullptr};
  binary_sensor::BinarySensor *charge_overcurrent_alarm_{nullptr};
  binary_sensor::BinarySensor *discharge_overcurrent_alarm_{nullptr};
  binary_sensor::BinarySensor *module_overvoltage_protection_{nullptr};
  binary_sensor::BinarySensor *module_undervoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_overvoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_undervoltage_protection_{nullptr};
  binary_sensor::BinarySensor *cell_overtemp_protection_{nullptr};
  binary_sensor::BinarySensor *cell_undertemp_protection_{nullptr};
  binary_sensor::BinarySensor *mosfet_overtemp_protection_{nullptr};
  binary_sensor::BinarySensor *charge_overcurrent_protection_{nullptr};
  binary_sensor::BinarySensor *discharge_overcurrent_protection_{nullptr};
  binary_sensor::BinarySensor *system_fault_protection_{nullptr};
#endif

  uint32_t cache_ttl_ms_{2500};
  uint32_t update_timeout_ms_{60000};
  uint32_t last_seen_ms_{0};
  uint32_t last_response_ms_{0};
  uint32_t last_request_ms_{0};
  uint32_t request_dedupe_window_ms_{250};
  uint32_t event_dedupe_window_ms_{2000};
  uint32_t response_timeout_ms_{1500};
  uint32_t link_timeout_ms_{60000};
  uint32_t last_link_activity_ms_{0};
  bool link_online_{false};
  bool passthrough_unknown_requests_{true};
  bool passthrough_battery_responses_{true};
  bool is_battery_port_{false};
  std::string rx_buffer_;
  std::string last_request_key_;
  std::string last_passthrough_originator_{"??"};
  std::string last_passthrough_command_{"??"};
  std::string last_battery_response_;
  std::string last_battery_event_;
  uint32_t last_battery_event_ms_{0};
  static std::vector<PylontechDualProxy *> active_instances_;
  static std::deque<PendingRequest> pending_requests_;
  text_sensor::TextSensor *last_battery_frame_sensor_{nullptr};
  text_sensor::TextSensor *last_inverter_request_sensor_{nullptr};
  text_sensor::TextSensor *last_battery_event_sensor_{nullptr};
  text_sensor::TextSensor *last_inverter_heartbeat_sensor_{nullptr};
  text_sensor::TextSensor *last_battery_snapshot_sensor_{nullptr};
  number::Number *ha_update_interval_number_{nullptr};
  number::Number *gsad_update_interval_number_{nullptr};
  number::Number *gscd_limits_update_interval_number_{nullptr};
  number::Number *module_analog_update_interval_number_{nullptr};
  number::Number *system_parameters_update_interval_number_{nullptr};
  number::Number *protocol_version_update_interval_number_{nullptr};
  number::Number *manufacturer_info_update_interval_number_{nullptr};
  number::Number *charge_management_update_interval_number_{nullptr};
  number::Number *module_serial_update_interval_number_{nullptr};
  number::Number *firmware_info_update_interval_number_{nullptr};
  std::map<std::string, uint32_t> last_ha_publish_ms_;
  binary_sensor::BinarySensor *battery_charging_sensor_{nullptr};
  binary_sensor::BinarySensor *battery_discharging_sensor_{nullptr};
  binary_sensor::BinarySensor *online_sensor_{nullptr};
  switch_::Switch *publish_raw_frames_switch_{nullptr};
  switch_::Switch *publish_decoded_replies_switch_{nullptr};

  static SharedBatterySnapshot shared_snapshot_;
};

}  // namespace pylontech_dual_proxy
}  // namespace esphome
