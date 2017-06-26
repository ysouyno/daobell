#ifndef HTTP_MULTI_THREADS_DOWNLOADER_H
#define HTTP_MULTI_THREADS_DOWNLOADER_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "url_parse.h"
#include "http_header_parse.h"
#include "log_wrapper.h"

class http_multi_threads_downloader
{
public:
  http_multi_threads_downloader() {}

  ~http_multi_threads_downloader()
  {
    if (sock_ > 0) {
      close(sock_);
    }

    if (fp_ > 0) {
      fclose(fp_);
    }
  }

  http_multi_threads_downloader(const std::string &url, size_t thread_count = 1) :
    url_(url),
    thread_count_(thread_count)
  {
    if (1 == thread_count_) {
      log_i("single thread\n");
      current_thread_index_ = 0;
    }
  }

  int init();

private:
  std::string url_;
  url_parse up_;
  size_t thread_count_;
  size_t thread_offset_beg_;
  size_t thread_offset_end_;
  size_t thread_file_size_;
  size_t file_size_;
  size_t current_thread_index_; // from 0 to thread_count_
  int sock_;
  FILE *fp_;
};

#endif /* HTTP_MULTI_THREADS_DOWNLOADER_H */
