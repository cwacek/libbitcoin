
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "peer.h"
#include "net.h"

struct peer * peer_connect(struct net *net, const char *node, int port)
{
  struct addrinfo hints;
  struct addrinfo *result;
  struct peer *peer;
  char sport[6];
  int error;
  int s;

  memset(&hints, 0, sizeof(struct addrinfo));

  snprintf(sport, 6, "%d", port);
  hints.ai_flags = AI_ALL;

  error = getaddrinfo(node, sport, &hints, &result);
  if(error != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return NULL;
  }

  s = socket(result->ai_family, SOCK_STREAM, IPPROTO_TCP);
  if(s < 0)
    return NULL;

  if(connect(s, result->ai_addr, result->ai_addrlen) < 0)
    return NULL;

  /* if we got an IPv4 address we need to convert it to mapped v4 format */
  /* this is only for internal representation now when the connection 
   * has been established */
  if(net_map_address(&result) != 0)
    return NULL;

  peer = peer_new(net, s, (struct sockaddr_in6*)result->ai_addr);
  freeaddrinfo(result);

  if(peer_start_handshake(peer) != 0)
    return NULL;

  return peer;
}

struct peer * peer_new(struct net *net, int socket, struct sockaddr_in6 *addr)
{
  struct peer *peer;

  assert(net != NULL);
  assert(addr != NULL);
  assert(socket > 0);

#ifdef DEBUG
  {
    char node[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &addr->sin6_addr, node, INET6_ADDRSTRLEN);
    printf("debug: new peer %s\n", node);
  }
#endif

  peer = calloc(1, sizeof(struct peer));
  peer->socket = socket;
  peer->net = net;
  peer->state = PEER_STATE_INITIALIZED;

  memcpy(&peer->addr, addr, sizeof(struct sockaddr_in6));

  return peer;
}

void peer_free(struct peer *peer)
{
  free(peer);
}

int peer_start_handshake(struct peer *peer)
{
  assert(peer->state == PEER_STATE_INITIALIZED);

  peer->state = PEER_STATE_HANDSHAKE; 

  /* send our version message */
  if(net_send_version(peer, 0) != 0)
    return -1;

  return 0;
}
