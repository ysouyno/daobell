#ifndef DNLD_FILE_H
#define DNLD_FILE_H

#include <pthread.h>
#include <sys/mman.h>
#include <string>

struct file_mem
{
  void *mem;
  size_t size;
};

struct dnld_file
{
  pthread_mutex_t mutex;
  std::string path;
  unsigned size;
  unsigned char *data; // memory pointer
};

dnld_file *dnld_file_create_and_open(const std::string &path, unsigned size);

int dnld_file_close_and_free(dnld_file *file);

#endif /* DNLD_FILE_H */
