#ifndef HTTP_MULTI_THREADS_DOWNLOADER_H
#define HTTP_MULTI_THREADS_DOWNLOADER_H

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include "http_url_parser.h"
#include "http_header_parser.h"
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

  http_multi_threads_downloader(const std::string &url, size_t thread_count = 1, size_t thread_index = 0) :
    url_(url),
    thread_count_(thread_count),
    current_thread_index_(thread_index),
    fp_(NULL),
    downloaded_size_(0)
  {
    if (1 == thread_count_) {
      log_t("single thread\n");
      current_thread_index_ = 0;
    }
  }

  size_t get_current_thread_index()
  {
    return current_thread_index_;
  }

  int download_it();

  size_t get_file_size()
  {
    if (init() < 0) {
      log_e("init() error\n");
      return -1;
    }

    std::string header;
    gen_request_header(header);
    send_request_query_http_header();

    return file_size_;
  }

  size_t get_downloaded_size() const
  {
    return downloaded_size_;
  }

  const std::string &get_dest_file_name_temp() const
  {
    return dest_file_name_temp_;
  }

  const std::string &get_dest_file_name() const
  {
    return dest_file_name_;
  }

private:
  int init();
  int connect_server();
  int gen_request_header(std::string &out);
  int send_request_query_http_header();
  int gen_range_header(std::string &out);
  int send_range_request(const std::string &header);
  int get_dest_file_name(std::string &out);
  int gen_file_name_temp(std::string &out);

  std::string url_;
  std::string file_name_temp_;
  std::string dest_file_name_;
  std::string dest_file_name_temp_;
  http_url_parser hup_;
  size_t thread_count_;
  size_t thread_offset_beg_;
  size_t thread_offset_end_;
  size_t thread_file_size_;
  size_t file_size_;
  size_t current_thread_index_; // from 0 to thread_count_ - 1
  int sock_;
  FILE *fp_;
  size_t downloaded_size_;
};

#endif /* HTTP_MULTI_THREADS_DOWNLOADER_H */
