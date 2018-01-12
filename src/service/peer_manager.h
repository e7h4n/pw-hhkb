#ifndef PW_HHKB_PEER_MANAGER_H
#define PW_HHKB_PEER_MANAGER_H

#include <sched.h>
#include <peer_manager/peer_manager_types.h>

void peerManager_init();

void peerManager_getPeers(uint16_t *p_peers, __uint32_t *p_size);

void peerManager_refreshWhitelist();

pm_peer_id_t peerManager_currentPeerId();

#endif //PW_HHKB_PEER_MANAGER_H