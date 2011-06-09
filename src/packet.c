#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include "peer.h"
#include "net.h"

#include "packet-int.h"

static struct packet * packet_new(struct net *net, const char *command, 
    size_t payload, int flags)
{
  size_t length;
  struct packet *packet = calloc(1, sizeof(struct packet));

  /* minimal header */
  length = 4 + 12 + 4;

  if((flags & NET_NO_CHECKSUM) == 0)
    length += 4;

  length += payload;

  void *data = calloc(1, length);

  packet->data = data;
  packet->offset = 0;
  packet->length = length;
  packet->flags = flags;

  add_uint32_t(packet, net->magic);
  add_string(packet, command, 12);
  add_uint32_t(packet, payload);

  if((flags & NET_NO_CHECKSUM) == 0)
  {
    /* add dummy checksum for now */
    add_uint32_t(packet, ~0);
  }

  return packet;
}

void packet_free(struct packet *packet)
{
  free(packet->data);
  free(packet);
}

int packet_send(struct peer *peer, struct packet *packet)
{
  assert(packet->length == packet->offset);

  if(send(peer->socket, packet->data, packet->length, 0) != packet->length)
    return -1;

  return 0;
}

struct packet * packet_version(struct peer *peer, int start_height)
{
  struct net *net;
  struct packet *packet;

  net = peer->net;

  BEGIN_PACKET(packet, net, "version", NET_NO_CHECKSUM);

  /* version */
  ADD_FIELD(packet, uint32_t, 32200);
  /* services */
  ADD_FIELD(packet, uint64_t, net->services);
  /* timestamp */
  ADD_FIELD(packet, uint64_t, time(NULL));
  /* addr_me */
  ADD_FIELD_NETADDR(packet, &net->addr);
  /* addr_you */
  ADD_FIELD_NETADDR(packet, &peer->addr);
  /* nonce */
  ADD_FIELD(packet, uint64_t, time(NULL));
  /* sub version */
  ADD_FIELD_STRINGZ(packet, net->version);
  /* start height */
  ADD_FIELD(packet, uint32_t, start_height);

  END_PACKET;

  return packet;
}

static void add_uint32_t(struct packet *packet, uint32_t value)
{
  *(uint32_t*)(packet->data + packet->offset) = value;
  packet->offset += sizeof(uint32_t);
}

static void add_uint64_t(struct packet *packet, uint64_t value)
{
  *(uint64_t*)(packet->data + packet->offset) = value;
  packet->offset += sizeof(uint64_t);
}

static void add_stringz(struct packet *packet, const char *value)
{
  size_t len;
  len = strlen(value) + 1;
  memcpy(packet->data + packet->offset, value, len);
  packet->offset += strlen(value) + 1;
}

static void add_string(struct packet *packet, const char *value, size_t length)
{
  memcpy(packet->data + packet->offset, value, length);
  packet->offset += length;
}

static void add_net_addr(struct packet *packet, struct net_addr *value)
{
  /* TODO */
  packet->offset += 8 + 16 + 2;
}

