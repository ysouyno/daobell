#ifndef DNLD_FILE_H
#define DNLD_FILE_H

#include <pthread.h>
#include <sys/mman.h>
#include <string>
#include <memory>

struct file_mem
{
  void *mem;
  size_t size;
};

class dnld_file
{
 public:
  dnld_file() = default;
  ~dnld_file() = default;

 public:
  int create_and_open(const std::string &path, unsigned size);
  int close_and_free();
  std::shared_ptr<file_mem> get_file_mem();
  int complete();

 private:
  pthread_mutex_t mutex_;
  std::string path_;
  unsigned size_;
  unsigned char *data_;
  std::shared_ptr<file_mem> file_mem_;
};

#endif /* DNLD_FILE_H */
