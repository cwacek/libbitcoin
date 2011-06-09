
#ifndef __HEADER_PACKET_INTERNAL__
#define __HEADER_PACKET_INTERNAL__

struct packet
{
  uint8_t *data;
  size_t offset;
  size_t length;
  int flags;
};


static void add_uint32_t(struct packet *packet, uint32_t value);
static void add_uint64_t(struct packet *packet, uint64_t value);
static void add_stringz(struct packet *packet, const char *value);
static void add_string(struct packet *packet, const char *value, size_t length);
static void add_net_addr(struct packet *packet, struct net_addr *value);

#define BEGIN_PACKET(packet, net, cmd, flags) \
  do \
  { \
    int i; \
    int length; \
    for(i = 0; i < 2; i++) \
    { \
      if(i == 0) \
        length = 0; \
      else if(i == 1) \
        packet = packet_new(net, cmd, length, flags);

#define END_PACKET \
    } \
  } while(0)

#define ADD_FIELD(packet, type, value) \
      if(i == 0) \
        length += sizeof(type); \
      else if(i == 1) \
        add_##type(packet, (type)value);
 
#define ADD_FIELD_NETADDR(packet, value) \
      if(i == 0) \
        length += 26; \
      else if(i == 1) \
        add_net_addr(packet, value);
 
#define ADD_FIELD_STRINGZ(packet, value) \
      if(i == 0) \
        length += strlen(value) + 1; \
      else if(i == 1) \
        add_stringz(packet, value);
  
#define ADD_FIELD_STRING(packet, value, size) \
      if(i == 0) \
        length += size; \
      else if(i == 1) \
        add_string(packet, value, size);

#endif

