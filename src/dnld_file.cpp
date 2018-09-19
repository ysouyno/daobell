#include "dnld_file.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <assert.h>

int dnld_file::create_and_open(const std::string &path, unsigned size)
{
  int fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  if (ftruncate(fd, size)) {
    perror("ftruncate");
    close(fd);
    return -1;
  }

  struct stat file_stat = {0};
  if (fstat(fd, &file_stat)) {
    perror("fstat");
    close(fd);
    return -1;
  }

  void *mem = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
  if (MAP_FAILED == mem) {
    perror("mmap");
    close(fd);
    return -1;
  }

  pthread_mutex_init(&mutex_, NULL);
  size_ = size;
  data_ = (unsigned char *)mem;
  path_ = path + ".incomplete";

  file_mem_ = std::make_shared<file_mem>();
  file_mem_->mem = data_;
  file_mem_->size = size_;

  rename(path.c_str(), path_.c_str());

  close(fd);
  return 0;
}

int dnld_file::close_and_free()
{
  int ret = 0;

  if (munmap(data_, size_)) {
    perror("munmap");
    ret = -1;
  }

  pthread_mutex_destroy(&mutex_);

  return ret;
}

std::shared_ptr<file_mem> dnld_file::get_file_mem()
{
  return file_mem_;
}

int dnld_file::complete()
{
  std::string old_path = path_;

  size_t pos = path_.find(".incomplete");
  if (std::string::npos != pos) {
    path_ = path_.substr(0, pos);
    std::cout << "new path: " << path_ << std::endl;

    rename(old_path.c_str(), path_.c_str());

    close_and_free();
  }

  return 0;
}
