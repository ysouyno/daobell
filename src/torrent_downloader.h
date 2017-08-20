#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include "bencode_parser.h"
#include "torrent_helper.h"
#include "torrent_info.h"
#include "http_url_parser.h"
#include "log_wrapper.h"
#include "daobell_utils.h"

class torrent_downloader
{
public:
  torrent_downloader();
  ~torrent_downloader();

  void download_it(const std::string &torrent_file, const std::string &dest_dir);

private:
  int connect_tracker(pthread_t *tid, torrent_info *ti);
};

#endif /* TORRENT_DOWNLOADER_H */
