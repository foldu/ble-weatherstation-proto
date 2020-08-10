#include "main.h"
#include "bmx280.h"
#include "board.h"
#include "gatt_svr.h"
#include "hal/hal_timer.h"
#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "log.h"
#include "net/bluetil/ad.h"
#include "nimble_riot.h"
#include "os/os_cputime.h"
#include "periph/i2c.h"
#include "periph_conf.h"
#include "periph_cpu.h"
#include "periph_cpu_common.h"
#include "sensor.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "xtimer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// Primary BME280 I^2C address 76
#define BME280_I2C_PRIMARY 0x76
/// Secondary BME280 I^2C address 77
#define BME280_I2C_SECONDARY 0x77

static void start_advertise(void);
static int gap_event_cb(struct ble_gap_event *, void *);

int main(void)
{
    int rc = 0;
    xtimer_init();

    static ws_sensor_ctx_t sensor;
    const i2c_t i2c_bus = I2C_DEV(0);
    i2c_init(i2c_bus);
    rc = ws_sensor_ctx_init(
        &sensor,
        &(bmx280_params_t){
            .i2c_dev = i2c_bus,
            .i2c_addr = BME280_I2C_PRIMARY,
            .temp_oversample = BMX280_OSRS_X1,
            .press_oversample = BMX280_OSRS_X1,
            .humid_oversample = BMX280_OSRS_X1,
            .t_sb = BMX280_SB_0_5,
            .filter = BMX280_FILTER_OFF,
            .run_mode = BMX280_MODE_FORCED,
        },
        (timex_t){
            .seconds = 30,
            .microseconds = 0,
        });

    ws_gatt_set_sensor(&sensor);

    assert(!rc);

    LOG("Inited sensor");

    rc = ble_gatts_count_cfg(ws_gatt_svr_svcs);
    assert(!rc);
    rc = ble_gatts_add_svcs(ws_gatt_svr_svcs);
    assert(!rc);

    ble_svc_gap_device_name_set(DEVICE_NAME);
    ble_gatts_start();

    LOG("Started BLE GATTS");

    // TODO: use ble_gap_ext
    static uint8_t buf[BLE_HS_ADV_MAX_SZ];
    bluetil_ad_t ad;
    bluetil_ad_init_with_flags(&ad, buf, sizeof(buf), BLUETIL_AD_FLAGS_DEFAULT);
    bluetil_ad_add_name(&ad, DEVICE_NAME);
    ble_gap_adv_set_data(ad.buf, ad.pos);

    start_advertise();

    return 0;
}

static void start_advertise(void)
{
    const struct ble_gap_adv_params advertisement_params = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN,
        .itvl_min = BLE_GAP_ADV_ITVL_MS(1200),
        .itvl_max = BLE_GAP_ADV_ITVL_MS(1800),
    };

    const int rc = ble_gap_adv_start(
        nimble_riot_own_addr_type, NULL, BLE_HS_FOREVER, &advertisement_params, gap_event_cb, NULL);
    assert(!rc);
}

/// Callback triggered on GAP event.
static int gap_event_cb(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:
        LOG("Connect");
        if (event->connect.status) {
            start_advertise();
        }
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        LOG("Disconnect");
        start_advertise();
        break;
    }

    return 0;
}
