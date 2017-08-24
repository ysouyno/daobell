#ifndef FTP_DOWNLOADER_H
#define FTP_DOWNLOADER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <string>
#include "ftp_url_parser.h"
#include "log_wrapper.h"

#define BUFFER_SIZE 1024

class ftp_downloader
{
public:
  ftp_downloader() = default;
  ~ftp_downloader() = default;

  explicit ftp_downloader(const std::string &url);

  int download_it();

private:
  std::string url_;
};

#endif /* FTP_DOWNLOADER_H */
