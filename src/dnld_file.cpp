#include "dnld_file.h"
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>

dnld_file *dnld_file_create_and_open(const std::string &path, unsigned size)
{
  int fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);
  if (fd < 0) {
    perror("open");
    return NULL;
  }

  if (ftruncate(fd, size)) {
    perror("ftruncate");
    close(fd);
    return NULL;
  }

  struct stat file_stat = {0};
  if (fstat(fd, &file_stat)) {
    perror("fstat");
    close(fd);
    return NULL;
  }

  void *mem = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
  if (MAP_FAILED == mem) {
    perror("mmap");
    close(fd);
    return NULL;
  }

  dnld_file *file = new dnld_file;
  memset(file, 0, sizeof(dnld_file));

  pthread_mutex_init(&file->mutex, NULL);
  file->size = size;
  file->data = (unsigned char *)mem;
  file->path = path + ".incomplete";

  rename(path.c_str(), file->path.c_str());

  close(fd);

  std::cout << "create and open file success: " << file->path << std::endl;
  return file;
}

int dnld_file_close_and_free(dnld_file *file)
{
  int ret = 0;

  if (munmap(file->data, file->size)) {
    perror("munmap");
    ret = -1;
  }

  pthread_mutex_destroy(&file->mutex);
  delete(file);

  return ret;
}

void dnld_file_get_file_mem(const dnld_file *file, file_mem *out)
{
  out->mem = file->data;
  out->size = file->size;
}
