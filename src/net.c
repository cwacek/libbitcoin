
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

#include "packet.h"
#include "peer.h"
#include "net.h"

struct net * net_new(int network, const char *listen, const char *addr, 
    int port, services_t services, const char *version)
{
  struct addrinfo hints;
  struct addrinfo *result;
  struct net *net;
  char sport[6];
  int error;
  int s;

  memset(&hints, 0, sizeof(struct addrinfo));

  snprintf(sport, 6, "%d", port);
#ifndef IPV6
  hints.ai_family = AF_INET;
#endif
  hints.ai_flags = AI_ALL | AI_NUMERICSERV;

  error = getaddrinfo(listen, sport, &hints, &result);
  if(error != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return NULL;
  }

  s = socket(result->ai_family, SOCK_STREAM, IPPROTO_TCP);
  if(bind(s, result->ai_addr, result->ai_addrlen) < 0)
  {
    freeaddrinfo(result);
    return NULL;
  }

#ifdef DEBUG
  printf("debug: bound to %s:%d\n", listen, port);
#endif

  /* make sure we have an AF_INET6 address to advertise */
  result = net_map_address(addr, port);
  if(result == NULL)
    return NULL;

#ifdef DEBUG
  {
    char new_node[INET6_ADDRSTRLEN];
    struct sockaddr_in6 * sin6;
    sin6 = (struct sockaddr_in6*)result->ai_addr;
    inet_ntop(result->ai_family, &sin6->sin6_addr, new_node, INET6_ADDRSTRLEN);
    printf("debug: advertising as %s\n", new_node);
  }
#endif

  net = calloc(1, sizeof(struct net));

  net->magic = network;
  net->services = services;
  memcpy(&net->addr, result->ai_addr, sizeof(struct sockaddr_in6));
  net->socket = s;
  net->version = strdup(version);

  freeaddrinfo(result);

  return net;
}

void net_free(struct net *net)
{
  free(net->version);
  free(net);
}

struct addrinfo * net_map_address(const char *node, int port)
{
  struct addrinfo *result;
  struct addrinfo hints;
  char sport[6];
  int error;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;
  hints.ai_flags = AI_V4MAPPED | AI_NUMERICSERV;

  snprintf(sport, 6, "%d", port);
  error = getaddrinfo(node, sport, &hints, &result);
  if(error != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
    return NULL;
  }

#ifdef DEBUG
  {
    char new_node[INET6_ADDRSTRLEN];
    struct sockaddr_in6 * sin6;
    sin6 = (struct sockaddr_in6*)result->ai_addr;
    inet_ntop(result->ai_family, &sin6->sin6_addr, new_node, INET6_ADDRSTRLEN);
    printf("debug: mapped %s to %s\n", node, new_node);
  }
#endif

  assert(result->ai_family == AF_INET6);

  return result;
}

int net_send_version(struct peer *peer, int start_height)
{
  struct packet *packet;
  int ret;

  packet = packet_version(peer, start_height);
  ret = packet_send(peer, packet);
  packet_free(packet);

  return ret;
}
