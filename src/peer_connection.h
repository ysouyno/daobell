#ifndef PEER_CONNECTION_H
#define PEER_CONNECTION_H

#include "peer_info.h"
#include "torrent_info2.h"

struct peer_arg
{
  bool has_torrent;
  torrent_info2 *torrent;
  bool has_sockfd;
  int sockfd;
  peer_info peer;
};

int peer_connection_create(pthread_t *thread, peer_arg *arg);

#endif /* PEER_CONNECTION_H */
