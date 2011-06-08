
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>

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

static void * net_new_buffer(size_t length)
{
  struct net_header_slim *packet = calloc(1, length);

  packet->length = length - sizeof(struct net_header_slim);
  return packet;
}

static void net_free_buffer(void * buffer)
{
  free(buffer);
}

int net_send_version(struct peer *peer, int start_height)
{
  /* base for version packet */
  struct net_pkt_version
  {
    struct net_header_slim header;
    uint32_t version;
    uint64_t services;
    uint64_t timestamp;
    struct net_addr addr_me;
    struct net_addr addr_you;
    uint64_t nonce;
  } __attribute__((packed));

  struct net_pkt_version *packet;
  char *packet_subver;
  uint32_t *packet_start_height;
  struct net *net;
  int len;

  net = peer->net;
  len = sizeof(struct net_pkt_version) + strlen(net->version) + 5;

  packet = net_new_buffer(len);
  packet_subver = (char*)(packet+1);
  packet_start_height = (uint32_t*)(packet_subver+strlen(net->version)+1);

  /* TODO: portable LE */
  packet->header.magic = peer->net->magic;
  strncpy(packet->header.command, "version", 12);
  packet->version = 32200;
  packet->services = net->services;
  packet->timestamp = (uint64_t)time(NULL);
  packet->nonce = 0x1234;

  packet->addr_me.services = net->services;
  packet->addr_me.port = net->addr.sin6_port;
  memcpy(&packet->addr_me.addr, &net->addr.sin6_addr, 
      sizeof(struct in6_addr));
 
  /* we do not know what services the remote node has */
  packet->addr_you.services = 1;
  packet->addr_you.port = peer->addr.sin6_port;
  memcpy(&packet->addr_you.addr, &peer->addr.sin6_addr,
      sizeof(struct in6_addr));

  /* add sub-version */
  strcpy(packet_subver, net->version);

  /* add start height */
  *packet_start_height = start_height;

  if(send(peer->socket, packet, len, 0) != len)
    return -1;

  net_free_buffer(packet);
  return 0;
}

