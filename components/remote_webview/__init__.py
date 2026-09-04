import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import display, touchscreen, binary_sensor, sensor
from esphome.components.display import validate_rotation
from esphome.const import (
    CONF_ID, CONF_DISPLAY_ID, CONF_URL, CONF_ROTATION, CONF_TRIGGER_ID,
    DEVICE_CLASS_CONNECTIVITY, ENTITY_CATEGORY_DIAGNOSTIC, STATE_CLASS_MEASUREMENT, STATE_CLASS_TOTAL_INCREASING,
    UNIT_MILLISECOND,
)


CONF_DEVICE_ID = "device_id"
CONF_TOUCHSCREEN_ID = "touchscreen_id"
CONF_SERVER = "server"
CONF_TILE_SIZE = "tile_size"
CONF_FULL_FRAME_TILE_COUNT = "full_frame_tile_count"
CONF_FULL_FRAME_AREA_THRESHOLD = "full_frame_area_threshold"
CONF_FULL_FRAME_EVERY = "full_frame_every"
CONF_EVERY_NTH_FRAME = "every_nth_frame"
CONF_MIN_FRAME_INTERVAL = "min_frame_interval"
CONF_JPEG_QUALITY = "jpeg_quality"
CONF_MAX_BYTES_PER_MSG = "max_bytes_per_msg"
CONF_BIG_ENDIAN = "big_endian"
CONF_ON_CONNECT = "on_connect"
CONF_ON_DISCONNECT = "on_disconnect"
CONF_CONNECTED_SENSOR = "connected_sensor"
CONF_FPS_SENSOR = "fps_sensor"
CONF_FRAME_TIME_SENSOR = "frame_time_sensor"
CONF_RECONNECTS_SENSOR = "reconnects_sensor"
CONF_CHROMA = "chroma_subsampling"
CONF_SCREENCAST_FORMAT = "screencast_format"
CONF_SCREENCAST_QUALITY = "screencast_quality"
CONF_REDUCED_MOTION = "reduced_motion"
CONF_SCREENCAST_MODE = "screencast_mode"
CONF_RLE_MAX_RATIO = "rle_max_ratio"
CONF_HW_JPEG = "hw_jpeg"
CONF_LOSSLESS_MAX_RATIO = "lossless_max_ratio"
CONF_DEFLATE_LEVEL = "deflate_level"
CONF_MAX_INFLIGHT = "max_inflight"

_SERVER_RE = re.compile(
    r"^(?P<host>[A-Za-z0-9](?:[A-Za-z0-9\-\.]*[A-Za-z0-9])?)\:(?P<port>\d{1,5})$"
)

AUTO_LOAD = ["binary_sensor", "sensor"]
DEPENDENCIES = ["display"]

def validate_host_port(value):
    s = cv.string_strict(value).strip()
    m = _SERVER_RE.match(s)
    if not m:
        raise cv.Invalid("server must be in 'host:port' format (no IPv6, no trailing colon)")

    host = m.group("host")
    port = int(m.group("port"), 10)

    if not (1 <= port <= 65535):
        raise cv.Invalid("port must be between 1 and 65535")

    return f"{host}:{port}"

ns = cg.esphome_ns.namespace("remote_webview")
RemoteWebView = ns.class_("RemoteWebView", cg.Component)
OnConnectTrigger = ns.class_("OnConnectTrigger", automation.Trigger.template())
OnDisconnectTrigger = ns.class_("OnDisconnectTrigger", automation.Trigger.template())
RefreshAction = ns.class_("RefreshAction", automation.Action)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RemoteWebView),
        cv.GenerateID(CONF_DISPLAY_ID): cv.use_id(display.Display),
        cv.GenerateID(CONF_TOUCHSCREEN_ID): cv.use_id(touchscreen.Touchscreen),
        cv.Required(CONF_SERVER): validate_host_port,
        cv.Required(CONF_URL): cv.string,

        cv.Optional(CONF_DEVICE_ID): cv.string,
        cv.Optional(CONF_TILE_SIZE): cv.int_,
        cv.Optional(CONF_FULL_FRAME_TILE_COUNT): cv.int_,
        cv.Optional(CONF_FULL_FRAME_AREA_THRESHOLD): cv.float_,
        cv.Optional(CONF_FULL_FRAME_EVERY): cv.int_,
        cv.Optional(CONF_EVERY_NTH_FRAME): cv.int_,
        cv.Optional(CONF_MIN_FRAME_INTERVAL): cv.int_,
        cv.Optional(CONF_JPEG_QUALITY): cv.int_,
        cv.Optional(CONF_MAX_BYTES_PER_MSG): cv.int_,
        cv.Optional(CONF_BIG_ENDIAN): cv.boolean,
        cv.Optional(CONF_ROTATION): validate_rotation,
        cv.Optional(CONF_CHROMA): cv.one_of("444", "420", "4:4:4", "4:2:0", lower=True),
        cv.Optional(CONF_SCREENCAST_FORMAT): cv.one_of("png", "jpeg", lower=True),
        cv.Optional(CONF_SCREENCAST_QUALITY): cv.int_range(min=1, max=100),
        cv.Optional(CONF_REDUCED_MOTION): cv.boolean,
        cv.Optional(CONF_SCREENCAST_MODE): cv.one_of("stream", "ondemand", lower=True),
        cv.Optional(CONF_RLE_MAX_RATIO): cv.float_range(min=0.0, max=1.0),
        cv.Optional(CONF_HW_JPEG, default=True): cv.boolean,
        cv.Optional(CONF_LOSSLESS_MAX_RATIO, default=0.5): cv.float_range(min=0.0, max=1.0),
        cv.Optional(CONF_DEFLATE_LEVEL, default=6): cv.int_range(min=1, max=9),
        cv.Optional(CONF_MAX_INFLIGHT): cv.int_range(min=1, max=4),
        cv.Optional(CONF_ON_CONNECT): automation.validate_automation(
            {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OnConnectTrigger)}
        ),
        cv.Optional(CONF_ON_DISCONNECT): automation.validate_automation(
            {cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OnDisconnectTrigger)}
        ),
        cv.Optional(CONF_CONNECTED_SENSOR): binary_sensor.binary_sensor_schema(
            device_class=DEVICE_CLASS_CONNECTIVITY,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FPS_SENSOR): sensor.sensor_schema(
            unit_of_measurement="fps",
            accuracy_decimals=1,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_FRAME_TIME_SENSOR): sensor.sensor_schema(
            unit_of_measurement=UNIT_MILLISECOND,
            accuracy_decimals=0,
            state_class=STATE_CLASS_MEASUREMENT,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
        cv.Optional(CONF_RECONNECTS_SENSOR): sensor.sensor_schema(
            accuracy_decimals=0,
            state_class=STATE_CLASS_TOTAL_INCREASING,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        ),
    }
).extend(cv.COMPONENT_SCHEMA)

REFRESH_ACTION_SCHEMA = automation.maybe_simple_id(
    {cv.GenerateID(CONF_ID): cv.use_id(RemoteWebView)}
)


@automation.register_action("remote_webview.refresh", RefreshAction, REFRESH_ACTION_SCHEMA)
async def remote_webview_refresh_to_code(config, action_id, template_arg, args):
    paren = await cg.get_variable(config[CONF_ID])
    return cg.new_Pvariable(action_id, template_arg, paren)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    disp = await cg.get_variable(config[CONF_DISPLAY_ID])
    cg.add(var.set_display(disp))
    cg.add(var.set_server(config[CONF_SERVER]))
    cg.add(var.set_url(config[CONF_URL]))

    if CONF_TOUCHSCREEN_ID in config:
        ts = await cg.get_variable(config[CONF_TOUCHSCREEN_ID])
        cg.add(var.set_touchscreen(ts))

    if CONF_DEVICE_ID in config:
        cg.add(var.set_device_id(config[CONF_DEVICE_ID]))
    if CONF_TILE_SIZE in config:
        cg.add(var.set_tile_size(config[CONF_TILE_SIZE]))
    if CONF_FULL_FRAME_TILE_COUNT in config:
        cg.add(var.set_full_frame_tile_count(config[CONF_FULL_FRAME_TILE_COUNT]))
    if CONF_FULL_FRAME_AREA_THRESHOLD in config:
        cg.add(var.set_full_frame_area_threshold(config[CONF_FULL_FRAME_AREA_THRESHOLD]))
    if CONF_FULL_FRAME_EVERY in config:
        cg.add(var.set_full_frame_every(config[CONF_FULL_FRAME_EVERY]))
    if CONF_EVERY_NTH_FRAME in config:
        cg.add(var.set_every_nth_frame(config[CONF_EVERY_NTH_FRAME]))
    if CONF_MIN_FRAME_INTERVAL in config:
        cg.add(var.set_min_frame_interval(config[CONF_MIN_FRAME_INTERVAL]))
    if CONF_JPEG_QUALITY in config:
        cg.add(var.set_jpeg_quality(config[CONF_JPEG_QUALITY]))
    if CONF_MAX_BYTES_PER_MSG in config:
        cg.add(var.set_max_bytes_per_msg(config[CONF_MAX_BYTES_PER_MSG]))
    if CONF_BIG_ENDIAN in config:
        cg.add(var.set_big_endian(config[CONF_BIG_ENDIAN]))
    if CONF_ROTATION in config:
        cg.add(var.set_rotation(config[CONF_ROTATION]))
    if CONF_CHROMA in config:
        cg.add(var.set_chroma_subsampling(config[CONF_CHROMA].replace(":", "")))
    if CONF_SCREENCAST_FORMAT in config:
        cg.add(var.set_screencast_format(config[CONF_SCREENCAST_FORMAT]))
    if CONF_SCREENCAST_QUALITY in config:
        cg.add(var.set_screencast_quality(config[CONF_SCREENCAST_QUALITY]))
    if CONF_REDUCED_MOTION in config:
        cg.add(var.set_reduced_motion(config[CONF_REDUCED_MOTION]))
    if CONF_SCREENCAST_MODE in config:
        cg.add(var.set_screencast_mode(config[CONF_SCREENCAST_MODE]))
    if CONF_RLE_MAX_RATIO in config:
        cg.add(var.set_rle_max_ratio(config[CONF_RLE_MAX_RATIO]))
    cg.add(var.set_hw_jpeg(config[CONF_HW_JPEG]))
    cg.add(var.set_lossless_max_ratio(config[CONF_LOSSLESS_MAX_RATIO]))
    cg.add(var.set_deflate_level(config[CONF_DEFLATE_LEVEL]))
    if CONF_MAX_INFLIGHT in config:
        cg.add(var.set_max_inflight(config[CONF_MAX_INFLIGHT]))

    for conf in config.get(CONF_ON_CONNECT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
    for conf in config.get(CONF_ON_DISCONNECT, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    if CONF_CONNECTED_SENSOR in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_CONNECTED_SENSOR])
        cg.add(var.set_connected_sensor(bs))
    if CONF_FPS_SENSOR in config:
        s = await sensor.new_sensor(config[CONF_FPS_SENSOR])
        cg.add(var.set_fps_sensor(s))
    if CONF_FRAME_TIME_SENSOR in config:
        s = await sensor.new_sensor(config[CONF_FRAME_TIME_SENSOR])
        cg.add(var.set_frame_time_sensor(s))
    if CONF_RECONNECTS_SENSOR in config:
        s = await sensor.new_sensor(config[CONF_RECONNECTS_SENSOR])
        cg.add(var.set_reconnects_sensor(s))

    await cg.register_component(var, config)
