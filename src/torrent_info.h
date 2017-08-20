#ifndef TORRENT_INFO_H
#define TORRENT_INFO_H

#include <iostream>
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
  std::vector<std::pair<std::string, long long> > files_;
  long long files_size_;
};

#endif /* TORRENT_INFO_H */
