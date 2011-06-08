
#ifndef __HEADER_NET__
#define __HEADER_NET__

#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define NETWORK_MAIN 0xD9B4BEF9

#define NETWORK_SERVICE_NODE 0x01

struct peer;

typedef uint64_t services_t;

struct net
{
  int socket;
  uint32_t magic;
  services_t services;
  struct sockaddr_in6 addr;
  char *version;
};

struct net * net_new(int network, const char *addr, int port, 
    services_t services, const char *version);

void net_free(struct net *net);

/**
 * Converts an AF_INET family address to an IPv4 mapped IPv6 address.
 * If input has AF_INET6 family nothing is done.
 *
 * addr might be freed and reallocated. Caller must free addr.
 */
int net_map_address(struct addrinfo **info);

int net_send_version(struct peer *peer, int start_height);

/** packet generation **/

struct net_header_slim
{
  uint32_t magic;
  char command[12];
  uint32_t length;
} __attribute__((packed));

struct net_header
{
  uint32_t magic;
  char command[12];
  uint32_t length;
  uint32_t checksum;
} __attribute__((packed));

struct net_addr
{
  uint64_t services;
  struct in6_addr addr;
  uint16_t port;
} __attribute__((packed));

#endif

