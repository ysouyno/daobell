#ifndef PEER_CONNECTION_H
#define PEER_CONNECTION_H

#include "peer_info.h"

struct torrent_info2;

struct peer_conn
{
  pthread_t tid;
  peer_info peer;
};

struct peer_arg
{
  bool has_torrent;
  torrent_info2 *torrent;
  bool has_sockfd;
  int sockfd;
  peer_info peer;
};

#endif /* PEER_CONNECTION_H */
