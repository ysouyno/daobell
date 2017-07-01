#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <pthread.h>
#include <errno.h>
#include "http_multi_threads_downloader.h"
#include "log_wrapper.h"

class http_downloader_wrapper
{
public:
  http_downloader_wrapper() {}
  ~http_downloader_wrapper() {}

  http_downloader_wrapper(const std::string &url, const size_t thread_count) :
    url_(url),
    thread_count_(thread_count)
  {
  }

  void download_file();

private:
  void merge_file();

private:
  std::string url_;
  size_t thread_count_;
  std::shared_ptr<http_multi_threads_downloader> sp_hmtd_;
  std::vector<std::shared_ptr<http_multi_threads_downloader> > vec_sp_hmtd_;
};
