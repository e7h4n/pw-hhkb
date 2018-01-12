//
// Created by Ethan Zhang on 12/01/2018.
//

#include "peer_manager.h"

#include <sdk_common.h>
#include <peer_manager/peer_manager.h>
#include <nrf_log.h>
#include <common/ble_conn_state.h>
#include <src/main.h>
#include <fds.h>
#include <src/config.h>

#include "advertising.h"

static pm_peer_id_t m_peer_id; /**< Device reference handle to the current bonded central. */
static pm_peer_id_t m_whitelist_peers[BLE_GAP_WHITELIST_ADDR_MAX_COUNT]; /**< List of peers currently in the whitelist. */
static uint32_t m_whitelist_peer_cnt; /**< Number of peers currently in the whitelist. */
static bool m_is_wl_changed; /**< Indicates if the whitelist has been changed since last time it has been updated in the Peer Manager. */

static void eventHandler(pm_evt_t const *p_evt);

static void whitelistInit();

void peerManager_init() {
    whitelistInit();

    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond = SEC_PARAM_BOND;
    sec_param.mitm = SEC_PARAM_MITM;
    sec_param.lesc = SEC_PARAM_LESC;
    sec_param.keypress = SEC_PARAM_KEYPRESS;
    sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob = SEC_PARAM_OOB;
    sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc = 1;
    sec_param.kdist_own.id = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(eventHandler);
    APP_ERROR_CHECK(err_code);
}

pm_peer_id_t peerManager_currentPeerId() {
    return m_peer_id;
}

void peerManager_refreshWhitelist() {
    if (!m_is_wl_changed) {
        return;
    }

    APP_ERROR_CHECK(pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt));

    uint32_t err_code = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
    if (err_code != NRF_ERROR_NOT_SUPPORTED) {
        APP_ERROR_CHECK(err_code);
    }

    m_is_wl_changed = false;
}

void peerManager_getPeers(uint16_t *p_peers, __uint32_t *p_size) {
    uint16_t peer_id;
    __uint32_t peers_to_copy;

    peers_to_copy = (*p_size < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) ?
                    *p_size : BLE_GAP_WHITELIST_ADDR_MAX_COUNT;

    peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    *p_size = 0;

    while ((peer_id != PM_PEER_ID_INVALID) && (peers_to_copy--)) {
        p_peers[(*p_size)++] = peer_id;
        peer_id = pm_next_peer_id_get(peer_id);
    }
}

static void eventHandler(pm_evt_t const *p_evt) {
    ret_code_t err_code;

    switch (p_evt->evt_id) {
        case PM_EVT_BONDED_PEER_CONNECTED: {
            NRF_LOG_INFO("Connected to a previously bonded device.\r\n");
        }
            break;

        case PM_EVT_CONN_SEC_SUCCEEDED: {
            NRF_LOG_INFO("Connection secured. Role: %d. conn_handle: %d, Procedure: %d\r\n",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);

            m_peer_id = p_evt->peer_id;

            // Note: You should check on what kind of white list policy your application should use.
            if (p_evt->params.conn_sec_succeeded.procedure == PM_LINK_SECURED_PROCEDURE_BONDING) {
                NRF_LOG_INFO("New Bond, add the peer to the whitelist if possible\r\n");
                NRF_LOG_INFO("\tm_whitelist_peer_cnt %d, MAX_PEERS_WLIST %d\r\n",
                             m_whitelist_peer_cnt + 1,
                             BLE_GAP_WHITELIST_ADDR_MAX_COUNT);

                if (m_whitelist_peer_cnt < BLE_GAP_WHITELIST_ADDR_MAX_COUNT) {
                    // Bonded to a new peer, add it to the whitelist.
                    m_whitelist_peers[m_whitelist_peer_cnt++] = m_peer_id;
                    m_is_wl_changed = true;
                }
            }
        }
            break;

        case PM_EVT_CONN_SEC_FAILED: {
            /* Often, when securing fails, it shouldn't be restarted, for security reasons.
             * Other times, it can be restarted directly.
             * Sometimes it can be restarted, but only after changing some Security Parameters.
             * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
             * Sometimes it is impossible, to secure the link, or the peer device does not support it.
             * How to handle this error is highly application dependent. */
        }
            break;

        case PM_EVT_CONN_SEC_CONFIG_REQ: {
            // Reject pairing request from an already bonded peer.
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        }
            break;

        case PM_EVT_STORAGE_FULL: {
            // Run garbage collection on the flash.
            err_code = fds_gc();
            if (err_code == FDS_ERR_BUSY || err_code == FDS_ERR_NO_SPACE_IN_QUEUES) {
                // Retry.
            } else {
                APP_ERROR_CHECK(err_code);
            }
        }
            break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED: {
            advertising_active();
        }
            break;

        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED: {
            // The local database has likely changed, send service changed indications.
            pm_local_database_has_changed();
        }
            break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED: {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        }
            break;

        case PM_EVT_PEER_DELETE_FAILED: {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        }
            break;

        case PM_EVT_PEERS_DELETE_FAILED: {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        }
            break;

        case PM_EVT_ERROR_UNEXPECTED: {
            // Assert.
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        }
            break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}

static void whitelistInit() {
    memset(m_whitelist_peers, PM_PEER_ID_INVALID, sizeof(m_whitelist_peers));
    m_whitelist_peer_cnt = (sizeof(m_whitelist_peers) / sizeof(pm_peer_id_t));

    peerManager_getPeers(m_whitelist_peers, &m_whitelist_peer_cnt);

    APP_ERROR_CHECK(pm_whitelist_set(m_whitelist_peers, m_whitelist_peer_cnt));

    uint32_t ret = pm_device_identities_list_set(m_whitelist_peers, m_whitelist_peer_cnt);
    if (ret != NRF_ERROR_NOT_SUPPORTED) {
        APP_ERROR_CHECK(ret);
    }

    m_is_wl_changed = false;
}

