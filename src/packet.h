
#ifndef __HEADER_PACKET__
#define __HEADER_PACKET__

#include "peer.h"

struct packet;

struct packet * packet_version(struct peer *peer, int start_height);

int packet_send(struct peer *peer, struct packet *packet);

void packet_free(struct packet *);

#endif
