#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <iostream>
#include <string>
#include <fstream>
#include "bencode_parser.h"
#include "torrent_helper.h"

class torrent_downloader
{
public:
  torrent_downloader();
  ~torrent_downloader();

  void download_it(const std::string &torrent_file, const std::string &dest_dir);
};

#endif /* TORRENT_DOWNLOADER_H */
