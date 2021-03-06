#ifndef PIECE_REQUEST_H
#define PIECE_REQUEST_H

#include "dnld_file.h"
#include "torrent_info.h"
#include <list>

struct block_request
{
  std::list<std::shared_ptr<file_mem> > file_mems;
  off_t begin;
  size_t len;
  bool completed;
};

struct piece_request
{
  unsigned piece_index;
  std::list<std::shared_ptr<block_request> > block_requests;
  unsigned blocks_left;
};

int piece_request_create(const torrent_info *torrent, unsigned index,
                         piece_request *out);
block_request *piece_request_block_at(const piece_request *request,
                                      off_t offset);

#endif /* PIECE_REQUEST_H */
