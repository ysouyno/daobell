#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include <netdb.h>
#include "bencode_parser.h"
#include "torrent_helper.h"
#include "torrent_info.h"
#include "http_url_parser.h"
#include "log_wrapper.h"
#include "utils.h"

class torrent_downloader
{
public:
  torrent_downloader();
  ~torrent_downloader();

  void download_it(const std::string &torrent_file, const std::string &dest_dir);

private:
  // int get_tracker_socket(const http_url_parser &hup);
  // int send_tracker_request(int sockfd, const char *request, size_t length);
  int connect_tracker(pthread_t *tid, torrent_info *ti);
};

#endif /* TORRENT_DOWNLOADER_H */
