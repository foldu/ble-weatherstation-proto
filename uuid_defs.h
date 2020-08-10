#ifndef WS_UUID_DEFS
#define WS_UUID_DEFS

#include "host/ble_uuid.h"

/**
 * @file
 * BLE service and characteristic UUIDs.
 * Everything not marked non-standard is declared in this document
 * https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
 */

/// Environmental sensing service UUID
extern const ble_uuid16_t BLE_GATT_SVC_ENVIRONMENTAL_SENSING;

/// Temperature characteristic UUID.
extern const ble_uuid16_t BLE_GATT_CHAR_TEMPERATURE;

/// Pressure characteristic UUID.
extern const ble_uuid16_t BLE_GATT_CHAR_PRESSURE;

/// Humidity characteristic UUID.
extern const ble_uuid16_t BLE_GATT_CHAR_HUMIDITY;

/// Non-standard UUID for pretty printing characteristic.
extern const ble_uuid128_t GATT_SVR_CHR_SENSOR_READ_UUID;

/// Non-standard UUID for pretty printing service.
extern const ble_uuid128_t GATT_SVR_SVC_SENSOR_UUID;

#endif
