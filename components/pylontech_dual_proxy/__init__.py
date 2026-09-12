import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor, binary_sensor, text_sensor, number
from esphome.const import CONF_ID

pylontech_dual_proxy_ns = cg.esphome_ns.namespace("esphome::pylontech_dual_proxy")
PylontechDualProxy = pylontech_dual_proxy_ns.class_(
    "PylontechDualProxy", cg.Component, uart.UARTDevice
)

CONF_CACHE_TTL = "cache_ttl"
CONF_UPDATE_TIMEOUT = "update_timeout"
CONF_BATTERY_PORT = "battery_port"
CONF_RESPONSE_TIMEOUT = "response_timeout"
CONF_STATE_OF_CHARGE = "state_of_charge"
CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"
CONF_TEMPERATURE = "temperature"
CONF_BATTERY_CHARGING = "battery_charging"
CONF_BATTERY_DISCHARGING = "battery_discharging"
CONF_LAST_BATTERY_SNAPSHOT = "last_battery_snapshot"
CONF_SOH = "state_of_health"
CONF_CYCLE_COUNT = "cycle_count"
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_TEMPERATURE = "max_temperature"
CONF_MIN_TEMPERATURE = "min_temperature"
CONF_MOSFET_TEMPERATURE = "mosfet_temperature"
CONF_MAX_MOSFET_TEMPERATURE = "max_mosfet_temperature"
CONF_MIN_MOSFET_TEMPERATURE = "min_mosfet_temperature"
CONF_BMS_TEMPERATURE = "bms_temperature"
CONF_MAX_BMS_TEMPERATURE = "max_bms_temperature"
CONF_MIN_BMS_TEMPERATURE = "min_bms_temperature"
CONF_MAX_VOLTAGE = "max_voltage"
CONF_MIN_VOLTAGE = "min_voltage"
CONF_MAX_CHARGE_CURRENT = "max_charge_current"
CONF_MAX_DISCHARGE_CURRENT = "max_discharge_current"
CONF_REQUESTED_FORCE_CHARGE = "requested_force_charge"

CONF_TOTAL_VOLTAGE_HIGH_ALARM = "total_voltage_high_alarm"
CONF_TOTAL_VOLTAGE_LOW_ALARM = "total_voltage_low_alarm"
CONF_CELL_VOLTAGE_HIGH_ALARM = "cell_voltage_high_alarm"
CONF_CELL_VOLTAGE_LOW_ALARM = "cell_voltage_low_alarm"
CONF_CELL_TEMP_HIGH_ALARM = "cell_temp_high_alarm"
CONF_CELL_TEMP_LOW_ALARM = "cell_temp_low_alarm"
CONF_MOSFET_TEMP_HIGH_ALARM = "mosfet_temp_high_alarm"
CONF_CELL_TEMP_IMBALANCE_ALARM = "cell_temp_imbalance_alarm"
CONF_CELL_IMBALANCE_ALARM = "cell_imbalance_alarm"
CONF_CHARGE_OVERCURRENT_ALARM = "charge_overcurrent_alarm"
CONF_DISCHARGE_OVERCURRENT_ALARM = "discharge_overcurrent_alarm"
CONF_MODULE_OVERVOLTAGE_PROTECTION = "module_overvoltage_protection"
CONF_MODULE_UNDERVOLTAGE_PROTECTION = "module_undervoltage_protection"
CONF_CELL_OVERVOLTAGE_PROTECTION = "cell_overvoltage_protection"
CONF_CELL_UNDERVOLTAGE_PROTECTION = "cell_undervoltage_protection"
CONF_CELL_OVERTEMP_PROTECTION = "cell_overtemp_protection"
CONF_CELL_UNDERTEMP_PROTECTION = "cell_undertemp_protection"
CONF_MOSFET_OVERTEMP_PROTECTION = "mosfet_overtemp_protection"
CONF_CHARGE_OVERCURRENT_PROTECTION = "charge_overcurrent_protection"
CONF_DISCHARGE_OVERCURRENT_PROTECTION = "discharge_overcurrent_protection"
CONF_SYSTEM_FAULT_PROTECTION = "system_fault_protection"
CONF_LAST_BATTERY_FRAME = "last_battery_frame"
CONF_LAST_INVERTER_REQUEST = "last_inverter_request"
CONF_LAST_BATTERY_EVENT = "last_battery_event"
CONF_LAST_INVERTER_HEARTBEAT = "last_inverter_heartbeat"
CONF_HA_UPDATE_INTERVAL = "ha_update_interval"
CONF_GSAD_UPDATE_INTERVAL = "gsad_update_interval"
CONF_GSCD_LIMITS_UPDATE_INTERVAL = "gscd_limits_update_interval"
CONF_MODULE_ANALOG_UPDATE_INTERVAL = "module_analog_update_interval"
CONF_SYSTEM_PARAMETERS_UPDATE_INTERVAL = "system_parameters_update_interval"
CONF_PROTOCOL_VERSION_UPDATE_INTERVAL = "protocol_version_update_interval"
CONF_MANUFACTURER_INFO_UPDATE_INTERVAL = "manufacturer_info_update_interval"
CONF_CHARGE_MANAGEMENT_UPDATE_INTERVAL = "charge_management_update_interval"
CONF_MODULE_SERIAL_UPDATE_INTERVAL = "module_serial_update_interval"
CONF_FIRMWARE_INFO_UPDATE_INTERVAL = "firmware_info_update_interval"
CONF_LINK_TIMEOUT = "link_timeout"
CONF_ONLINE = "online"

SENSOR_KEYS_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_BATTERY_PORT, default=False): cv.boolean,
        cv.Optional(CONF_RESPONSE_TIMEOUT, default="1500ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_LINK_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_ONLINE): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_STATE_OF_CHARGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_SOH): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_CYCLE_COUNT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_CELL_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_CELL_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_MOSFET_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_BMS_TEMPERATURE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MIN_VOLTAGE): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_CHARGE_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_MAX_DISCHARGE_CURRENT): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_REQUESTED_FORCE_CHARGE): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_BATTERY_CHARGING): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_BATTERY_DISCHARGING): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_TOTAL_VOLTAGE_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_TOTAL_VOLTAGE_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_VOLTAGE_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_VOLTAGE_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_LOW_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MOSFET_TEMP_HIGH_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_TEMP_IMBALANCE_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_IMBALANCE_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CHARGE_OVERCURRENT_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_DISCHARGE_OVERCURRENT_ALARM): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MODULE_OVERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MODULE_UNDERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_OVERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_UNDERVOLTAGE_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_OVERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CELL_UNDERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_MOSFET_OVERTEMP_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CHARGE_OVERCURRENT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_DISCHARGE_OVERCURRENT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_SYSTEM_FAULT_PROTECTION): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_LAST_BATTERY_FRAME): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LAST_INVERTER_REQUEST): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LAST_BATTERY_EVENT): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_LAST_INVERTER_HEARTBEAT): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_HA_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_GSAD_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_GSCD_LIMITS_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_MODULE_ANALOG_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_SYSTEM_PARAMETERS_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_PROTOCOL_VERSION_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_MANUFACTURER_INFO_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_CHARGE_MANAGEMENT_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_MODULE_SERIAL_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_FIRMWARE_INFO_UPDATE_INTERVAL): cv.use_id(number.Number),
        cv.Optional(CONF_LAST_BATTERY_SNAPSHOT): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_CACHE_TTL, default="2500ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_UPDATE_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
    }
)

CONFIG_SCHEMA = cv.ensure_list(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechDualProxy),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
    .extend(SENSOR_KEYS_SCHEMA)
)


async def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await uart.register_uart_device(var, conf)

        if CONF_STATE_OF_CHARGE in conf:
            cg.add(var.set_soc_sensor((await cg.get_variable(conf[CONF_STATE_OF_CHARGE]))))
        if CONF_VOLTAGE in conf:
            cg.add(var.set_voltage_sensor((await cg.get_variable(conf[CONF_VOLTAGE]))))
        if CONF_CURRENT in conf:
            cg.add(var.set_current_sensor((await cg.get_variable(conf[CONF_CURRENT]))))
        if CONF_TEMPERATURE in conf:
            cg.add(var.set_temperature_sensor((await cg.get_variable(conf[CONF_TEMPERATURE]))))
        if CONF_SOH in conf:
            cg.add(var.set_soh_sensor((await cg.get_variable(conf[CONF_SOH]))))
        if CONF_CYCLE_COUNT in conf:
            cg.add(var.set_cycle_count_sensor((await cg.get_variable(conf[CONF_CYCLE_COUNT]))))
        if CONF_MAX_TEMPERATURE in conf:
            cg.add(var.set_max_temperature_sensor((await cg.get_variable(conf[CONF_MAX_TEMPERATURE]))))
        if CONF_MIN_TEMPERATURE in conf:
            cg.add(var.set_min_temperature_sensor((await cg.get_variable(conf[CONF_MIN_TEMPERATURE]))))
        if CONF_MAX_CELL_VOLTAGE in conf:
            cg.add(var.set_max_cell_voltage_sensor((await cg.get_variable(conf[CONF_MAX_CELL_VOLTAGE]))))
        if CONF_MIN_CELL_VOLTAGE in conf:
            cg.add(var.set_min_cell_voltage_sensor((await cg.get_variable(conf[CONF_MIN_CELL_VOLTAGE]))))
        if CONF_MOSFET_TEMPERATURE in conf:
            cg.add(var.set_mosfet_temperature_sensor((await cg.get_variable(conf[CONF_MOSFET_TEMPERATURE]))))
        if CONF_MAX_MOSFET_TEMPERATURE in conf:
            cg.add(var.set_max_mosfet_temperature_sensor((await cg.get_variable(conf[CONF_MAX_MOSFET_TEMPERATURE]))))
        if CONF_MIN_MOSFET_TEMPERATURE in conf:
            cg.add(var.set_min_mosfet_temperature_sensor((await cg.get_variable(conf[CONF_MIN_MOSFET_TEMPERATURE]))))
        if CONF_BMS_TEMPERATURE in conf:
            cg.add(var.set_bms_temperature_sensor((await cg.get_variable(conf[CONF_BMS_TEMPERATURE]))))
        if CONF_MAX_BMS_TEMPERATURE in conf:
            cg.add(var.set_max_bms_temperature_sensor((await cg.get_variable(conf[CONF_MAX_BMS_TEMPERATURE]))))
        if CONF_MIN_BMS_TEMPERATURE in conf:
            cg.add(var.set_min_bms_temperature_sensor((await cg.get_variable(conf[CONF_MIN_BMS_TEMPERATURE]))))
        if CONF_MAX_VOLTAGE in conf:
            cg.add(var.set_max_voltage_sensor((await cg.get_variable(conf[CONF_MAX_VOLTAGE]))))
        if CONF_MIN_VOLTAGE in conf:
            cg.add(var.set_min_voltage_sensor((await cg.get_variable(conf[CONF_MIN_VOLTAGE]))))
        if CONF_MAX_CHARGE_CURRENT in conf:
            cg.add(var.set_max_charge_current_sensor((await cg.get_variable(conf[CONF_MAX_CHARGE_CURRENT]))))
        if CONF_MAX_DISCHARGE_CURRENT in conf:
            cg.add(var.set_max_discharge_current_sensor((await cg.get_variable(conf[CONF_MAX_DISCHARGE_CURRENT]))))
        if CONF_REQUESTED_FORCE_CHARGE in conf:
            cg.add(var.set_requested_force_charge((await cg.get_variable(conf[CONF_REQUESTED_FORCE_CHARGE]))))
        if CONF_BATTERY_CHARGING in conf:
            cg.add(var.set_battery_charging_sensor((await cg.get_variable(conf[CONF_BATTERY_CHARGING]))))
        if CONF_BATTERY_DISCHARGING in conf:
            cg.add(var.set_battery_discharging_sensor((await cg.get_variable(conf[CONF_BATTERY_DISCHARGING]))))
        if CONF_ONLINE in conf:
            cg.add(var.set_online_sensor((await cg.get_variable(conf[CONF_ONLINE]))))

        if CONF_TOTAL_VOLTAGE_HIGH_ALARM in conf:
            cg.add(var.set_total_voltage_high_alarm((await cg.get_variable(conf[CONF_TOTAL_VOLTAGE_HIGH_ALARM]))))
        if CONF_TOTAL_VOLTAGE_LOW_ALARM in conf:
            cg.add(var.set_total_voltage_low_alarm((await cg.get_variable(conf[CONF_TOTAL_VOLTAGE_LOW_ALARM]))))
        if CONF_CELL_VOLTAGE_HIGH_ALARM in conf:
            cg.add(var.set_cell_voltage_high_alarm((await cg.get_variable(conf[CONF_CELL_VOLTAGE_HIGH_ALARM]))))
        if CONF_CELL_VOLTAGE_LOW_ALARM in conf:
            cg.add(var.set_cell_voltage_low_alarm((await cg.get_variable(conf[CONF_CELL_VOLTAGE_LOW_ALARM]))))
        if CONF_CELL_TEMP_HIGH_ALARM in conf:
            cg.add(var.set_cell_temp_high_alarm((await cg.get_variable(conf[CONF_CELL_TEMP_HIGH_ALARM]))))
        if CONF_CELL_TEMP_LOW_ALARM in conf:
            cg.add(var.set_cell_temp_low_alarm((await cg.get_variable(conf[CONF_CELL_TEMP_LOW_ALARM]))))
        if CONF_MOSFET_TEMP_HIGH_ALARM in conf:
            cg.add(var.set_mosfet_temp_high_alarm((await cg.get_variable(conf[CONF_MOSFET_TEMP_HIGH_ALARM]))))
        if CONF_CELL_TEMP_IMBALANCE_ALARM in conf:
            cg.add(var.set_cell_temp_imbalance_alarm((await cg.get_variable(conf[CONF_CELL_TEMP_IMBALANCE_ALARM]))))
        if CONF_CELL_IMBALANCE_ALARM in conf:
            cg.add(var.set_cell_imbalance_alarm((await cg.get_variable(conf[CONF_CELL_IMBALANCE_ALARM]))))
        if CONF_CHARGE_OVERCURRENT_ALARM in conf:
            cg.add(var.set_charge_overcurrent_alarm((await cg.get_variable(conf[CONF_CHARGE_OVERCURRENT_ALARM]))))
        if CONF_DISCHARGE_OVERCURRENT_ALARM in conf:
            cg.add(var.set_discharge_overcurrent_alarm((await cg.get_variable(conf[CONF_DISCHARGE_OVERCURRENT_ALARM]))))
        if CONF_MODULE_OVERVOLTAGE_PROTECTION in conf:
            cg.add(var.set_module_overvoltage_protection((await cg.get_variable(conf[CONF_MODULE_OVERVOLTAGE_PROTECTION]))))
        if CONF_MODULE_UNDERVOLTAGE_PROTECTION in conf:
            cg.add(var.set_module_undervoltage_protection((await cg.get_variable(conf[CONF_MODULE_UNDERVOLTAGE_PROTECTION]))))
        if CONF_CELL_OVERVOLTAGE_PROTECTION in conf:
            cg.add(var.set_cell_overvoltage_protection((await cg.get_variable(conf[CONF_CELL_OVERVOLTAGE_PROTECTION]))))
        if CONF_CELL_UNDERVOLTAGE_PROTECTION in conf:
            cg.add(var.set_cell_undervoltage_protection((await cg.get_variable(conf[CONF_CELL_UNDERVOLTAGE_PROTECTION]))))
        if CONF_CELL_OVERTEMP_PROTECTION in conf:
            cg.add(var.set_cell_overtemp_protection((await cg.get_variable(conf[CONF_CELL_OVERTEMP_PROTECTION]))))
        if CONF_CELL_UNDERTEMP_PROTECTION in conf:
            cg.add(var.set_cell_undertemp_protection((await cg.get_variable(conf[CONF_CELL_UNDERTEMP_PROTECTION]))))
        if CONF_MOSFET_OVERTEMP_PROTECTION in conf:
            cg.add(var.set_mosfet_overtemp_protection((await cg.get_variable(conf[CONF_MOSFET_OVERTEMP_PROTECTION]))))
        if CONF_CHARGE_OVERCURRENT_PROTECTION in conf:
            cg.add(var.set_charge_overcurrent_protection((await cg.get_variable(conf[CONF_CHARGE_OVERCURRENT_PROTECTION]))))
        if CONF_DISCHARGE_OVERCURRENT_PROTECTION in conf:
            cg.add(var.set_discharge_overcurrent_protection((await cg.get_variable(conf[CONF_DISCHARGE_OVERCURRENT_PROTECTION]))))
        if CONF_SYSTEM_FAULT_PROTECTION in conf:
            cg.add(var.set_system_fault_protection((await cg.get_variable(conf[CONF_SYSTEM_FAULT_PROTECTION]))))
        if CONF_LAST_BATTERY_FRAME in conf:
            cg.add(var.set_last_battery_frame_sensor((await cg.get_variable(conf[CONF_LAST_BATTERY_FRAME]))))
        if CONF_LAST_INVERTER_REQUEST in conf:
            cg.add(var.set_last_inverter_request_sensor((await cg.get_variable(conf[CONF_LAST_INVERTER_REQUEST]))))
        if CONF_LAST_BATTERY_EVENT in conf:
            cg.add(var.set_last_battery_event_sensor((await cg.get_variable(conf[CONF_LAST_BATTERY_EVENT]))))
        if CONF_LAST_INVERTER_HEARTBEAT in conf:
            cg.add(var.set_last_inverter_heartbeat_sensor((await cg.get_variable(conf[CONF_LAST_INVERTER_HEARTBEAT]))))
        if CONF_LAST_BATTERY_SNAPSHOT in conf:
            cg.add(var.set_last_battery_snapshot_sensor((await cg.get_variable(conf[CONF_LAST_BATTERY_SNAPSHOT]))))

        for key, setter in (
            (CONF_HA_UPDATE_INTERVAL, "set_ha_update_interval_number"),
            (CONF_GSAD_UPDATE_INTERVAL, "set_gsad_update_interval_number"),
            (CONF_GSCD_LIMITS_UPDATE_INTERVAL, "set_gscd_limits_update_interval_number"),
            (CONF_MODULE_ANALOG_UPDATE_INTERVAL, "set_module_analog_update_interval_number"),
            (CONF_SYSTEM_PARAMETERS_UPDATE_INTERVAL, "set_system_parameters_update_interval_number"),
            (CONF_PROTOCOL_VERSION_UPDATE_INTERVAL, "set_protocol_version_update_interval_number"),
            (CONF_MANUFACTURER_INFO_UPDATE_INTERVAL, "set_manufacturer_info_update_interval_number"),
            (CONF_CHARGE_MANAGEMENT_UPDATE_INTERVAL, "set_charge_management_update_interval_number"),
            (CONF_MODULE_SERIAL_UPDATE_INTERVAL, "set_module_serial_update_interval_number"),
            (CONF_FIRMWARE_INFO_UPDATE_INTERVAL, "set_firmware_info_update_interval_number"),
        ):
            if key in conf:
                cg.add(getattr(var, setter)((await cg.get_variable(conf[key]))))

        cg.add(var.set_battery_port(conf[CONF_BATTERY_PORT]))
        cg.add(var.set_response_timeout(conf[CONF_RESPONSE_TIMEOUT]))
        cg.add(var.set_link_timeout(conf[CONF_LINK_TIMEOUT]))
        cg.add(var.set_cache_ttl(conf[CONF_CACHE_TTL]))
        cg.add(var.set_update_timeout(conf[CONF_UPDATE_TIMEOUT]))
