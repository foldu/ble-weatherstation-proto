#include "uuid_defs.h"

// see
// https://btprodspecificationrefs.blob.core.windows.net/assigned-values/16-bit%20UUID%20Numbers%20Document.pdf
// nit: Why does the bluetooth commitee host on windows.net?
const ble_uuid16_t BLE_GATT_SVC_ENVIRONMENTAL_SENSING = BLE_UUID16_INIT(0x181a);

const ble_uuid16_t BLE_GATT_CHAR_TEMPERATURE = BLE_UUID16_INIT(0x2a6e);

const ble_uuid16_t BLE_GATT_CHAR_PRESSURE = BLE_UUID16_INIT(0x2a6d);

const ble_uuid16_t BLE_GATT_CHAR_HUMIDITY = BLE_UUID16_INIT(0x2a6f);

// UUID: e7364bd3-a1c5-4924-847d-3a9cd6e343ef
const ble_uuid128_t GATT_SVR_SVC_SENSOR_UUID = BLE_UUID128_INIT(
    0xef,
    0x43,
    0xe3,
    0xd6,
    0x9c,
    0x3a,
    0x7d,
    0x84,
    0x24,
    0x49,
    0xc5,
    0xa1,
    0xd3,
    0x4b,
    0x36,
    0xe7);

// UUID: 001e6351-80a5-4eeb-905f-d6c43268cfdc
const ble_uuid128_t GATT_SVR_CHR_SENSOR_READ_UUID = BLE_UUID128_INIT(
    0xdc,
    0xcf,
    0x68,
    0x32,
    0xc4,
    0xd6,
    0x5f,
    0x90,
    0xeb,
    0x4e,
    0xa5,
    0x80,
    0x51,
    0x63,
    0x1e,
    0x0);
