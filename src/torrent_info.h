#ifndef TORRENT_INFO_H
#define TORRENT_INFO_H

#include <string>
#include <vector>

struct torrent_info
{
  torrent_info();
  ~torrent_info();

  std::vector<std::string> announce_list_;
  long long creation_date_;
  long long piece_length_;
  std::string info_hash_;
};

#endif /* TORRENT_INFO_H */
