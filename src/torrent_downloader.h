#ifndef TORRENT_DOWNLOADER_H
#define TORRENT_DOWNLOADER_H

#include <iostream>
#include <string>
#include <fstream>
#include <pthread.h>
#include <netdb.h>
#include <sys/select.h>
#include "bencode_parser.h"
#include "torrent_helper.h"
#include "torrent_info.h"
#include "http_url_parser.h"
#include "log_wrapper.h"
#include "utils.h"
#include "http_header_parser.h"
#include "torrent_info.h"
#include "bencode_value_base.h"
#include "peer_message.h"

struct peer_thread_arg
{
  torrent_info *ti_;
  std::pair<std::string, uint16_t> *peer_;
  bool has_torrent_;
  int sockfd_;
};

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
