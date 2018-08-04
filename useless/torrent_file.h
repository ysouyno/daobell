#ifndef TORRENT_FILE_H
#define TORRENT_FILE_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>

struct TorrentFileInfo
{
  int fd;
  int size;
  unsigned char *data;
};

class TorrentFile
{
 public:
  TorrentFile();
  ~TorrentFile();

 public:
  int open_torrent_file(const char *metafile);
  int close_torrent_file(TorrentFileInfo *file_info);
  TorrentFileInfo *get_file_info();

 private:
  TorrentFileInfo *file_info_;
};

#endif /* TORRENT_FILE_H */
