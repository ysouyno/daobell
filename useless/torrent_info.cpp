#include "torrent_info.h"

torrent_info::torrent_info() :
  creation_date_(0),
  piece_length_(0),
  files_size_(0)
{
}

torrent_info::~torrent_info()
{
}
