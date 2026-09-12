import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, text_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

pylontech_uart_tap_ns = cg.esphome_ns.namespace("esphome::pylontech_uart_tap")
PylontechUartTap = pylontech_uart_tap_ns.class_("PylontechUartTap", cg.Component, uart.UARTDevice)

CONF_PEER_ID = "peer_id"
CONF_TAP_NAME = "tap_name"
CONF_LAST_FRAME = "last_frame"

CONFIG_SCHEMA = cv.ensure_list(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PylontechUartTap),
            cv.Required(CONF_PEER_ID): cv.use_id(PylontechUartTap),
            cv.Required(CONF_TAP_NAME): cv.string,
            cv.Optional(CONF_LAST_FRAME): cv.use_id(text_sensor.TextSensor),
        }
    )
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await uart.register_uart_device(var, conf)
        cg.add(var.set_tap_name(conf[CONF_TAP_NAME]))
        if CONF_LAST_FRAME in conf:
            cg.add(var.set_last_frame_sensor(await cg.get_variable(conf[CONF_LAST_FRAME])))

    for conf in config:
        var = await cg.get_variable(conf[CONF_ID])
        peer = await cg.get_variable(conf[CONF_PEER_ID])
        cg.add(var.set_peer(peer))
