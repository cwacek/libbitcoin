
#include <stdio.h>

#include "peer.h"
#include <errno.h>

int main(int argc, char *argv[])
{
  struct net *net;
  struct peer *peer;

  net = net_new(NETWORK_MAIN, "213.132.119.37", 8331, NETWORK_SERVICE_NODE, "libbitcoin 0.0.1");
  if(net == NULL)
  {
    perror("net_new");
  }

  peer = peer_connect(net, "x-group.se", 8333);
  if(peer == NULL)
  {
    perror("connect");
  }

  peer_free(peer);
  net_free(net);
  return 0;
}
