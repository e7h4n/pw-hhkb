//
// Created by Ethan Zhang on 12/01/2018.
//

#define NRF_LOG_MODULE_NAME "ADVERTISING"
#include <nrf_log.h>

#include "advertising.h"

#include <peer_manager/peer_manager.h>
#include <ble_advertising/ble_advertising.h>
#include <common/ble_srv_common.h>
#include <src/config.h>
#include "peer_manager.h"

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static void errorHandler(uint32_t nrf_error);
static void advertising_eventHandler(ble_adv_evt_t ble_adv_evt);

void advertising_active() {
    APP_ERROR_CHECK(ble_advertising_start(BLE_ADV_MODE_FAST));
}

void advertising_init() {
    peerManager_init();

    uint8_t adv_flags;
    ble_advdata_t advdata;
    ble_adv_modes_config_t options;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    adv_flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advdata.name_type = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;
    advdata.flags = adv_flags;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids = m_adv_uuids;

    memset(&options, 0, sizeof(options));
    options.ble_adv_whitelist_enabled = true;
    options.ble_adv_directed_enabled = true;
    options.ble_adv_directed_slow_enabled = false;
    options.ble_adv_directed_slow_interval = 0;
    options.ble_adv_directed_slow_timeout = 0;
    options.ble_adv_fast_enabled = true;
    options.ble_adv_fast_interval = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout = APP_ADV_FAST_TIMEOUT;
    options.ble_adv_slow_enabled = true;
    options.ble_adv_slow_interval = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout = APP_ADV_SLOW_TIMEOUT;

    APP_ERROR_CHECK(ble_advertising_init(&advdata,
                                         NULL,
                                         &options,
                                         (ble_advertising_evt_handler_t const) advertising_eventHandler,
                                         errorHandler));
}

static void errorHandler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

static void advertising_eventHandler(ble_adv_evt_t ble_adv_evt) {
    uint32_t err_code;

    switch (ble_adv_evt) {
        case BLE_ADV_EVT_DIRECTED:
            NRF_LOG_INFO("BLE_ADV_EVT_DIRECTED\r\n");
            break; //BLE_ADV_EVT_DIRECTED

        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("BLE_ADV_EVT_FAST\r\n");
            break; //BLE_ADV_EVT_FAST

        case BLE_ADV_EVT_SLOW:
            NRF_LOG_INFO("BLE_ADV_EVT_SLOW\r\n");
            break; //BLE_ADV_EVT_SLOW

        case BLE_ADV_EVT_FAST_WHITELIST:
            NRF_LOG_INFO("BLE_ADV_EVT_FAST_WHITELIST\r\n");
            break; //BLE_ADV_EVT_FAST_WHITELIST

        case BLE_ADV_EVT_SLOW_WHITELIST:
            NRF_LOG_INFO("BLE_ADV_EVT_SLOW_WHITELIST\r\n");
            break; //BLE_ADV_EVT_SLOW_WHITELIST

        case BLE_ADV_EVT_IDLE:
            // TODO: enter sleep mode
            break; //BLE_ADV_EVT_IDLE

        case BLE_ADV_EVT_WHITELIST_REQUEST: {
            ble_gap_addr_t whitelist_addrs[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t whitelist_irks[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            uint32_t addr_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            uint32_t irk_cnt = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

            err_code = pm_whitelist_get(whitelist_addrs, &addr_cnt,
                                        whitelist_irks, &irk_cnt);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_DEBUG("pm_whitelist_get returns %d addr in whitelist and %d irk whitelist\r\n",
                          addr_cnt,
                          irk_cnt);

            // Apply the whitelist.
            err_code = ble_advertising_whitelist_reply(whitelist_addrs, addr_cnt,
                                                       whitelist_irks, irk_cnt);
            APP_ERROR_CHECK(err_code);
        }
            break; //BLE_ADV_EVT_WHITELIST_REQUEST

        case BLE_ADV_EVT_PEER_ADDR_REQUEST: {
            pm_peer_data_bonding_t peer_bonding_data;

            // Only Give peer address if we have a handle to the bonded peer.
            if (peerManager_currentPeerId() != PM_PEER_ID_INVALID) {
                err_code = pm_peer_data_bonding_load(peerManager_currentPeerId(), &peer_bonding_data);
                if (err_code != NRF_ERROR_NOT_FOUND) {
                    APP_ERROR_CHECK(err_code);

                    ble_gap_addr_t *p_peer_addr = &(peer_bonding_data.peer_ble_id.id_addr_info);
                    err_code = ble_advertising_peer_addr_reply(p_peer_addr);
                    APP_ERROR_CHECK(err_code);
                }
            }
        }
            break; //BLE_ADV_EVT_PEER_ADDR_REQUEST

        default:
            break;
    }
}
