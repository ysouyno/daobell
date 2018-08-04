#include "torrent_file.h"

TorrentFile::TorrentFile()
{
  file_info_ = new TorrentFileInfo;
}

TorrentFile::~TorrentFile()
{
  if (file_info_) {
    delete file_info_;
  }
}

int TorrentFile::open_torrent_file(const char *metafile)
{
  int fd = open(metafile, /*O_RDWD*/02);
  if (-1 == fd) {
    perror("open");
    return -1;
  }

  struct stat file_stat;
  if (-1 == fstat(fd, &file_stat)) {
    perror("fstat");
    return -1;
  }

  void *pmap = NULL;
  pmap = mmap(NULL, file_stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (MAP_FAILED == pmap) {
    perror("mmap");
    return -1;
  }

  file_info_->fd = fd;
  file_info_->size = file_stat.st_size;
  file_info_->data = (unsigned char *)pmap;

  return 0;
}

int TorrentFile::close_torrent_file(TorrentFileInfo *file_info)
{
  assert(file_info);

  if (-1 == munmap(file_info->data, file_info->size)) {
    perror("munmap");
    return -1;
  }

  if (-1 == close(file_info->fd)) {
    perror("close");
    return -1;
  }

  return 0;
}

TorrentFileInfo *TorrentFile::get_file_info()
{
  return file_info_;
}
