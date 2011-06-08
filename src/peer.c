
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

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

#ifndef IPV6
  hints.ai_family = AF_INET;
#endif
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

  freeaddrinfo(result);

  /* if we got an IPv4 address we need to convert it to mapped v4 format */
  /* this is only for internal representation now when the connection 
   * has been established */
  peer = peer_new(net, s, node, port);

  if(peer_start_handshake(peer) != 0)
    return NULL;

  return peer;
}

struct peer * peer_new(struct net *net, int socket, const char *host,
    int port)
{
  struct peer *peer;
  struct addrinfo *addr;

  assert(net != NULL);
  assert(host != NULL);
  assert(socket > 0);

  addr = net_map_address(host, port);
  if(addr == NULL)
    return NULL;

#ifdef DEBUG
  {
    char new_node[INET6_ADDRSTRLEN];
    struct sockaddr_in6 * sin6;
    sin6 = (struct sockaddr_in6*)addr->ai_addr;
    inet_ntop(addr->ai_family, &sin6->sin6_addr, new_node, INET6_ADDRSTRLEN);
    printf("debug: new peer %s\n", new_node);
  }
#endif

  peer = calloc(1, sizeof(struct peer));
  peer->socket = socket;
  peer->net = net;
  peer->state = PEER_STATE_INITIALIZED;

  memcpy(&peer->addr, addr->ai_addr, sizeof(struct sockaddr_in6));

  return peer;
}

void peer_free(struct peer *peer)
{
  close(peer->socket);
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
