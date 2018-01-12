//
// Created by Ethan Zhang on 12/01/2018.
//

#include <sched.h>
#include <peer_manager/peer_manager_types.h>

#ifndef PW_HHKB_CONNECTION_H
#define PW_HHKB_CONNECTION_H

#endif //PW_HHKB_CONNECTION_H

void peerManager_init();

void peerManager_getPeers(uint16_t *p_peers, __uint32_t *p_size);

pm_peer_id_t peerManager_currentPeerId();

void peerManager_refreshWhitelist();