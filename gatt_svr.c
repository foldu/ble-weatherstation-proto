#include "gatt_svr.h"
#include "cpu.h"
#include "host/ble_att.h"
#include "host/ble_gatt.h"
#include "host/ble_uuid.h"
#include "log.h"
#include "main.h"
#include "nimble/ble.h"
#include "os/os_mbuf.h"
#include "periph_cpu.h"
#include "periph_cpu_common.h"
#include "sensor.h"
#include "string.h"
#include "util.h"

static ws_sensor_ctx_t *gatt_svr_sensor;

void ws_gatt_set_sensor(ws_sensor_ctx_t *ctx) { gatt_svr_sensor = ctx; }

// https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.temperature.xml
static int
output_temp(const ws_sensor_values_t *values, ws_sensor_read_result res, struct os_mbuf *mbuf)
{
    (void)res;
    const int16_t temp = values->temperature;
    return os_mbuf_append(mbuf, &temp, sizeof(temp));
}

// https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.pressure.xml
static int
output_pressure(const ws_sensor_values_t *values, ws_sensor_read_result res, struct os_mbuf *mbuf)
{
    (void)res;
    const uint32_t pressure = values->pressure * 10;
    return os_mbuf_append(mbuf, &pressure, sizeof(pressure));
}

// https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.humidity.xml
static int
output_hum(const ws_sensor_values_t *values, ws_sensor_read_result res, struct os_mbuf *mbuf)
{
    (void)res;
    const uint16_t hum = values->humidity;
    // TODO: decimal exponent is -2/0.01 see if this needs adjustment
    return os_mbuf_append(mbuf, &hum, sizeof(hum));
}

static int
output_pretty(const ws_sensor_values_t *values, ws_sensor_read_result res, struct os_mbuf *mbuf)
{
    char buf[64] = {0};
    int bytes_written = 1;
    switch (res) {
    case WS_SENSOR_ERROR:
        bytes_written = snprintf(buf, sizeof(buf), "Sensor is broken");
        break;
    case WS_SENSOR_ON_FIRE:
        LOG("Sensor is returning garbage values");
        // fallthru
    case WS_SENSOR_OK:
        bytes_written = snprintf(
            buf,
            sizeof(buf),
            SENSOR_TEXT_FORMAT_STRING,
            values->temperature,
            values->humidity,
            values->pressure);
        break;
    }

    if (bytes_written < 0) {
        LOG("Failed to write sensor output to buffer");
    } else if ((size_t)bytes_written > sizeof(buf)) {
        LOG("Sensor pretty output truncated");
    }

    return os_mbuf_append(mbuf, buf, bytes_written);
}

static int gatt_svr_chr_sensor_read(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctx,
    void *arg)
{
    (void)attr_handle;
    (void)conn_handle;
    (void)arg;

    LOG("Reading sensor");

    if (ctx->op != BLE_GATT_ACCESS_OP_READ_CHR) {
        return BLE_ERR_UNSUPPORTED;
    }

    struct table_entry {
        const ble_uuid_t *uuid;
        int (*fn)(const ws_sensor_values_t *, ws_sensor_read_result, struct os_mbuf *);
    };

    const struct table_entry uuid_table[] = {
        {
            .uuid = &GATT_SVR_CHR_SENSOR_READ_UUID.u,
            .fn = output_pretty,
        },
        {
            .uuid = &BLE_GATT_CHAR_TEMPERATURE.u,
            .fn = output_temp,
        },
        {
            .uuid = &BLE_GATT_CHAR_HUMIDITY.u,
            .fn = output_hum,
        },
        {
            .uuid = &BLE_GATT_CHAR_PRESSURE.u,
            .fn = output_pressure,
        },
    };

    for (size_t i = 0; i < STATIC_ARRAY_SIZE(uuid_table); i++) {
        const struct table_entry *entry = &uuid_table[i];
        if (!ble_uuid_cmp(ctx->chr->uuid, entry->uuid)) {
            ws_sensor_values_t values = {0};
            ws_sensor_read_result res = ws_sensor_ctx_read(gatt_svr_sensor, &values);
            return entry->fn(&values, res, ctx->om);
        }
    }

    LOG("Unhandled uuid");
    return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_chr_battery_status(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctx,
    void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;

    LOG("Battery status accessed");

    // kind of battery is currently unknown so we just pretend the battery is
    // always full
    const uint8_t battery_percentage = 100;

    return os_mbuf_append(ctx->om, &battery_percentage, sizeof(battery_percentage));
}

static int gatt_svr_chr_device_info(
    uint16_t conn_handle,
    uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctx,
    void *arg)
{
    (void)conn_handle;
    (void)attr_handle;
    (void)arg;
    const char *msg;

    switch (ble_uuid_u16(ctx->chr->uuid)) {
    case BLE_GATT_CHAR_MANUFACTURER_NAME:
        msg = MANUFACTURER_NAME;
        break;
    case BLE_GATT_CHAR_MODEL_NUMBER_STR:
        msg = MODEL_NUMBER;
        break;
    case BLE_GATT_CHAR_SERIAL_NUMBER_STR:
        msg = SERIAL_NUMBER;
        break;
    case BLE_GATT_CHAR_FW_REV_STR:
        msg = GIT_COMMIT;
        break;
    case BLE_GATT_CHAR_HW_REV_STR:
        msg = HW_REV;
        break;
    default:
        return BLE_ATT_ERR_UNLIKELY;
    }

    return os_mbuf_append(ctx->om, msg, strlen(msg));
}

/// GATT services definition.
const struct ble_gatt_svc_def ws_gatt_svr_svcs[] = {
    /// Environmental sensing GATT service.
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &BLE_GATT_SVC_ENVIRONMENTAL_SENSING.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &BLE_GATT_CHAR_TEMPERATURE.u,
                    .access_cb = gatt_svr_chr_sensor_read,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = &BLE_GATT_CHAR_HUMIDITY.u,
                    .access_cb = gatt_svr_chr_sensor_read,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = &BLE_GATT_CHAR_PRESSURE.u,
                    .access_cb = gatt_svr_chr_sensor_read,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0},
            },
    },

    /// Service for pretty printed output.
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &GATT_SVR_SVC_SENSOR_UUID.u,
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = &GATT_SVR_CHR_SENSOR_READ_UUID.u,
                    .access_cb = gatt_svr_chr_sensor_read,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0},
            },
    },

    // device info
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_GATT_SVC_DEVINFO),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_MANUFACTURER_NAME),
                    .access_cb = gatt_svr_chr_device_info,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_MODEL_NUMBER_STR),
                    .access_cb = gatt_svr_chr_device_info,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_SERIAL_NUMBER_STR),
                    .access_cb = gatt_svr_chr_device_info,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_FW_REV_STR),
                    .access_cb = gatt_svr_chr_device_info,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_HW_REV_STR),
                    .access_cb = gatt_svr_chr_device_info,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0},
            },
    },

    // battery status
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_GATT_SVC_BAS),
        .characteristics =
            (struct ble_gatt_chr_def[]){
                {
                    .uuid = BLE_UUID16_DECLARE(BLE_GATT_CHAR_BATTERY_LEVEL),
                    .access_cb = gatt_svr_chr_battery_status,
                    .flags = BLE_GATT_CHR_F_READ,
                },
                {0},
            },
    },
    {0},
};
