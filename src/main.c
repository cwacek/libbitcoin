
#include <stdio.h>

#include "peer.h"
#include <errno.h>

int main(int argc, char *argv[])
{
  struct net *net;
  struct peer *peer;

  net = net_new(NETWORK_MAIN, "10.11.32.36", "79.136.61.194", 8331, NETWORK_SERVICE_NODE, "libbitcoin 0.0.1");
  if(net == NULL)
  {
    perror("net_new");
    return 1;
  }

  peer = peer_connect(net, "213.132.119.34", 8333);
  if(peer == NULL)
  {
    perror("connect");
    return 1;
  }

  while(1);

  peer_free(peer);
  net_free(net);
  return 0;
}
