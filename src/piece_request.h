#ifndef PIECE_REQUEST_H
#define PIECE_REQUEST_H

#include "dnld_file.h"
#include <list>

struct block_request
{
  std::list<file_mem> file_mems;
  off_t begin;
  size_t len;
  bool completed;
};

struct piece_request
{
  unsigned piece_index;
  std::list<block_request> block_requests;
  unsigned blocks_left;
};

#endif /* PIECE_REQUEST_H */
