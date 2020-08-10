#include "bmx280.h"
#include "mutex.h"
#include "string.h"

#include "log.h"
#include "sensor.h"
#include "xtimer.h"

#define BME280_TEMP_ERR_VAL INT16_MIN

const timex_t NOT_MEASURED = (timex_t){
    .seconds = 0,
    .microseconds = 0,
};

static inline ws_sensor_read_result read_sensor(bmx280_t *sensor, ws_sensor_values_t *values)
{
    values->temperature = bmx280_read_temperature(sensor);
    if (values->temperature == BME280_TEMP_ERR_VAL) {
        return WS_SENSOR_ERROR;
    }

    const int16_t deg_c_90 = 9000;
    if (values->temperature >= deg_c_90) {
        return WS_SENSOR_ON_FIRE;
    }

    values->pressure = bmx280_read_pressure(sensor);
    values->pressure *= 10;

    values->humidity = bme280_read_humidity(sensor);

    return WS_SENSOR_OK;
}

int ws_sensor_ctx_init(
    ws_sensor_ctx_t *ctx,
    const bmx280_params_t *bme_280_cfg,
    const timex_t remeasure_time)
{
    // NOTE: it's correct to initialize mutex_t with 0
    memset(ctx, 0, sizeof(ws_sensor_ctx_t));
    ctx->last_sample = NOT_MEASURED;
    ctx->remeasure_time = remeasure_time;
    return bmx280_init(&ctx->sensor, bme_280_cfg);
}

/// Substract @p t1 from @p t0 with overflow wrapping.
static timex_t timex_wrapping_sub(const timex_t t0, const timex_t t1)
{
    return (timex_t){
        .seconds = t0.seconds - t1.seconds,
        .microseconds = t0.seconds - t1.seconds,
    };
}

ws_sensor_read_result ws_sensor_ctx_read(ws_sensor_ctx_t *ctx, ws_sensor_values_t *ret)
{
    mutex_lock(&ctx->mutex);

    timex_t now;
    xtimer_now_timex(&now);

    ws_sensor_read_result res = WS_SENSOR_OK;

    if (timex_cmp(ctx->last_sample, NOT_MEASURED) == 0 ||
        timex_cmp(timex_wrapping_sub(now, ctx->last_sample), ctx->remeasure_time) >= 0) {
        res = read_sensor(&ctx->sensor, ret);
        switch (res) {
        case WS_SENSOR_OK:
            memcpy(&ctx->values, ret, sizeof(ws_sensor_values_t));
            LOG("Read new values from sensor: " SENSOR_TEXT_FORMAT_STRING,
                ret->temperature,
                ret->humidity,
                ret->pressure);
            // only set last sample on ok so next read will cause a retry
            ctx->last_sample = now;
            break;

        case WS_SENSOR_ERROR:
            break;

        case WS_SENSOR_ON_FIRE:
            // use the old value and hope it stops returning garbage
            // after getting polled again
            memcpy(ret, &ctx->values, sizeof(ws_sensor_values_t));
            break;
        }
    } else {
        memcpy(ret, &ctx->values, sizeof(ws_sensor_values_t));
    }

    mutex_unlock(&ctx->mutex);

    return res;
}
