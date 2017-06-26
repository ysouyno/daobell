#ifndef HTTP_DOWNLOADER_H
#define HTTP_DOWNLOADER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iomanip>
#include <fstream>
#include "url_parse.h"
#include "http_header_parse.h"
#include "log_wrapper.h"

class http_downloader
{
 public:
  http_downloader();
  ~http_downloader();
  http_downloader(const std::string &url);

  int download_it();

 private:
  int init();
  int gen_request();
  int gen_dest_file_name();

 private:
  std::string url_;
  std::string request_;
  std::string dest_file_name_;
  int sock_;
  url_parse up_;
};

#endif /* HTTP_DOWNLOADER_H */
