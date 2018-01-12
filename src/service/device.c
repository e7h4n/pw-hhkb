//
// Created by Ethan Zhang on 12/01/2018.
//

#include <sched.h>
#include <ble_services/ble_dis/ble_dis.h>
#include <src/config.h>
#include <memory.h>
#include <app_error.h>

/**@brief Function for initializing Device Information Service.
 */
void device_init() {
    uint32_t err_code;
    ble_dis_init_t dis_init_obj;
    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id = PNP_ID_VENDOR_ID;
    pnp_id.product_id = PNP_ID_PRODUCT_ID;
    pnp_id.product_version = PNP_ID_PRODUCT_VERSION;

    memset(&dis_init_obj, 0, sizeof(dis_init_obj));

    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}
