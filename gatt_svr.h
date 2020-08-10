#ifndef WS_GATT_SVR_H
#define WS_GATT_SVR_H

#include "host/ble_gatt.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "main.h"
#include "net/ble.h"
#include "net/bluetil/ad.h"
#include "sensor.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "uuid_defs.h"

/**
 * @file
 * BLE GATT services.
 */

/// GATT services definition.
extern const struct ble_gatt_svc_def ws_gatt_svr_svcs[];

/// Set the used sensor context used by @ref ws_gatt_svr_svcs.
/// @warning This _must_ be called before the ws_gatt_svr_svcs are used.
void ws_gatt_set_sensor(ws_sensor_ctx_t *);

#endif
