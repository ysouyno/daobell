#ifndef PEER_INFO_H
#define PEER_INFO_H

#include <netinet/ip.h>

struct peer_info
{
  char peer_id[20];
  union
  {
    struct sockaddr_storage sas;
    struct sockaddr sa;
    struct sockaddr_in sa_in;
    struct sockaddr_in6 sa_in6;
  } addr;
};

struct peer_conn
{
  pthread_t peer_tid;
  peer_info peer;
};

#endif /* PEER_INFO_H */
