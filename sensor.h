#ifndef WEATHERSTATION_SENSOR_H
#define WEATHERSTATION_SENSOR_H

#include "bmx280.h"
#include "ds18.h"
#include "mutex.h"
#include "timex.h"

/**
 * @file
 * Abstracted temperature/pressure/humidity sensor.
 */

/// A pretty format string for sensor output.
#define SENSOR_TEXT_FORMAT_STRING "T: %d H: %u P: %lu"

/**
 * Values read from the sensor.
 */
typedef struct {
    /// Temperature in degrees celsius with a precision of 2.
    int16_t temperature;
    /// Pressure in Pascal.
    uint32_t pressure;
    /// Relative humidity with a precision of 2.
    uint16_t humidity;
} ws_sensor_values_t;

/// Sensor sampling context.
typedef struct {
    /// @private
    bmx280_t sensor;
    /// @private
    /// Mutex for thread synchronization.
    mutex_t mutex;
    /// @private
    /// Time the sensor was last sampled.
    timex_t last_sample;
    /// @private
    /// Cached sensor values.
    ws_sensor_values_t values;
    /// @private
    /// Time to remeasure.
    timex_t remeasure_time;
} ws_sensor_ctx_t;

/// Result returned from reading the sensor.
typedef enum {
    /// Measurement had no errors.
    WS_SENSOR_OK,
    /// The sensor is broken.
    WS_SENSOR_ERROR,
    /// The sensor returned garbage.
    WS_SENSOR_ON_FIRE,
} ws_sensor_read_result;

/**
 * Initializes a sensor context.
 *
 * This will initialize the sensor with the configuration given in @p bme_280_cfg
 * and sets the time the @p ctx caches values.
 * @returns Return value of bmx_280_init, if not equal to zero this means an error occured.
 */
int ws_sensor_ctx_init(
    ws_sensor_ctx_t *ctx,
    const bmx280_params_t *bme_280_cfg,
    const timex_t remeasure_time);

/**
 * Reads from the sensor.
 *
 * If the remeasure time of @p ctx has expired, reads from the sensor and places the
 * new value into @p values. Otherwise writes the old cached measurement to values.
 * @remarks This function is thread safe and internally synchronized.
 */
ws_sensor_read_result ws_sensor_ctx_read(ws_sensor_ctx_t *ctx, ws_sensor_values_t *values);

#endif
