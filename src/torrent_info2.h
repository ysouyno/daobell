#ifndef TORRENT_INFO2_H
#define TORRENT_INFO2_H

#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_dictionary.h"
#include "bencode_list.h"
#include "bencode_encoder.h"
#include "bencode_value_safe_cast.h"
#include "dnld_file.h"
#include "peer_connection.h"
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

struct torrent_info2
{
  std::string announce;
  std::string comment;
  std::string created_by;
  uint32_t creation_date;
  unsigned piece_length;
  std::vector<dnld_file *> files;
  char info_hash[20];
  pthread_t tracker_tid;
  unsigned max_peers;
  std::vector<std::string> pieces;

  struct
  {
    boost::dynamic_bitset<> pieces_state;
    std::list<peer_conn> peer_connections;
    unsigned pieces_left;
    unsigned uploaded;
    unsigned downloaded;
    bool completed;
  } sh;

  pthread_mutex_t sh_mutex;
};

torrent_info2 *torrent_init(bencode_value_ptr meta, const std::string &destdir);

#endif /* TORRENT_INFO2_H */
