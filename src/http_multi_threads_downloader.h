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

  http_multi_threads_downloader(const std::string &url, size_t thread_count = 1, size_t thread_index = 0) :
    url_(url),
    thread_count_(thread_count),
    current_thread_index_(thread_index),
    fp_(NULL)
  {
    if (1 == thread_count_) {
      log_i("single thread\n");
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
    if (1 == thread_count_) {
      return file_size_;
    }
    else {
      return thread_offset_beg_ - thread_offset_end_;
    }
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
