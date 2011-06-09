
#ifndef __HEADER_NET__
#define __HEADER_NET__

#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define NETWORK_MAIN 0xD9B4BEF9

#define NETWORK_SERVICE_NODE 0x01

#define NET_NO_CHECKSUM 0x01

struct peer;

typedef uint64_t services_t;

struct net_addr
{
  uint64_t services;
  struct in6_addr addr;
  uint16_t port;
};

struct net
{
  int socket;
  uint32_t magic;
  services_t services;
  struct net_addr addr;
  char *version;
};

struct net * net_new(int network, const char *listen, const char *addr, 
    int port, services_t services, const char *version);

void net_free(struct net *net);

struct addrinfo * net_map_address(const char *node, int port);

int net_send_version(struct peer *peer, int start_height);

/** packet generation **/
#endif

