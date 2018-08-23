#ifndef TRACKER_ANNOUNCE_H
#define TRACKER_ANNOUNCE_H

#include <stdint.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include "bencode_value_base.h"

typedef std::shared_ptr<bencode_value_base> bencode_base_ptr;

enum torrent_event{
  TORRENT_EVENT_STARTED,
  TORRENT_EVENT_COMPLETED,
  TORRENT_EVENT_STOPPED
};

enum {
  REQUEST_HAS_IP = (1 << 0),
  REQUEST_HAS_NUMWANT = (1 << 1),
  REQUEST_HAS_NO_PEER_ID = (1 << 2),
  REQUEST_HAS_COMPACT = (1 << 3),
  REQUEST_HAS_KEY = (1 << 4),
  REQUEST_HAS_TRACKER_ID = (1 << 5),
  REQUEST_HAS_EVENT = (1 << 6)
};

struct tracker_announce_req
{
  char has;
  char info_hash[20];
  char peer_id[20];
  uint16_t port;
  unsigned long uploaded;
  unsigned long downloaded;
  unsigned long left;
  bool compact;
  bool no_peer_id;
  torrent_event event;
  struct sockaddr_in ip;
  unsigned numwant;
  char *key;
  char *trackerid;
};

struct tracker_announce_resp
{
  char has;
  std::string failure_reason;
  std::string warning_message;
  unsigned interval;
  unsigned min_interval;
  std::string tracker_id;
  unsigned complete;
  unsigned incomplete;
  bencode_base_ptr peers;
};

#endif /* TRACKER_ANNOUNCE_H */
