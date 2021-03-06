#ifndef TORRENT_INFO_H
#define TORRENT_INFO_H

#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_int.h"
#include "bencode_dict.h"
#include "bencode_list.h"
#include "bencode_encoder.h"
#include "bencode_value_safe_cast.h"
#include "dnld_file.h"
#include "peer_info.h"
#include "sha1.h"
#include <pthread.h>
#include <string>
#include <vector>
#include <list>
#include <boost/dynamic_bitset.hpp>
#include <map>
#include <sys/stat.h>

#define DEFAULT_MAX_PEERS 50

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

enum {
  PIECE_STATE_NOT_REQUESTED,
  PIECE_STATE_REQUESTED,
  PIECE_STATE_HAVE
};

struct torrent_info
{
  std::string announce;
  std::string comment;
  std::string created_by;
  uint32_t creation_date;
  unsigned piece_length;
  std::vector<std::shared_ptr<dnld_file> > files;
  char info_hash[20];
  pthread_t tracker_tid;
  unsigned max_peers;
  std::vector<std::string> pieces;

  struct
  {
    std::vector<int> pieces_state;
    std::list<peer_conn> peer_connections;
    unsigned pieces_left;
    unsigned uploaded;
    unsigned downloaded;
    bool completed;
  } sh;

  pthread_mutex_t sh_mutex;
};

torrent_info *torrent_init(bencode_value_ptr meta, const std::string &destdir);
int torrent_make_bitfield(const torrent_info *torrent,
                          boost::dynamic_bitset<> *out);
int torrent_next_request(torrent_info *torrent,
                         boost::dynamic_bitset<> *peer_have,
                         unsigned *out);
bool torrent_sha1_verify(const torrent_info *torrent, unsigned index);
int torrent_complete(torrent_info *torrent);

#endif /* TORRENT_INFO_H */
