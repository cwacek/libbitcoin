
#ifndef __HEADER_PEER__
#define __HEADER_PEER__

#include <netinet/in.h>

#include "net.h"

#define PEER_STATE_DISCONNECTED 0
#define PEER_STATE_INITIALIZED 1
#define PEER_STATE_HANDSHAKE 2
#define PEER_STATE_CONNECTED 3

struct peer
{
  int socket;
  struct sockaddr_in6 addr;
  int state;

  struct net *net;
};

struct peer * peer_new(struct net *net, int socket, struct sockaddr_in6 *addr);

void peer_free(struct peer *peer);

struct peer * peer_connect(struct net *net, const char *node, int ipv6);

int peer_start_handshake(struct peer *peer);

#endif
