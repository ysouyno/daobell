#include "piece_request.h"
#include "peer_msg.h"

void skip_until_index(const torrent_info2 *torrent, unsigned index,
                      off_t *offset, unsigned &files_vec_index)
{
  std::cout << "enter skip_until_index" << std::endl;

  size_t skip = torrent->piece_length * index;
  std::cout << "skip: " << skip << std::endl;

  for (unsigned i = 0; i < torrent->files.size() && skip > 0; ++i) {
    assert(torrent->files[i]);

    file_mem mem;
    dnld_file_get_file_mem(torrent->files[i], &mem);

    // this is the last file to skip
    if (mem.size > skip) {
      *offset = skip;
      files_vec_index = i;
      return;
    }
    else {
      skip -= mem.size;
    }
  }
}

block_request *next_block_request(const torrent_info2 *torrent,
                                  off_t *offset,
                                  size_t *left,
                                  size_t piece_len,
                                  unsigned files_vec_index)
{
  std::cout << "enter next_block_request" << std::endl;

  if (0 == *left) {
    std::cout << "left is zero" << std::endl;
    return NULL;
  }

  block_request *ret = new block_request;
  if (!ret) {
    return NULL;
  }

  ret->begin = piece_len - *left;
  ret->len = 0;
  ret->completed = false;

  unsigned curr_size = 0;

  for (unsigned i = files_vec_index;
       i < torrent->files.size() && curr_size < PEER_REQUEST_SIZE;
       ++i) {
    dnld_file *file = torrent->files[i];
    assert(file);

    file_mem mem;
    dnld_file_get_file_mem(file, &mem);

    mem.mem = ((char *)mem.mem + *offset);
    mem.size -= *offset;

    if (mem.size > PEER_REQUEST_SIZE - curr_size) {
      mem.size = PEER_REQUEST_SIZE - curr_size;
      *offset += mem.size;
    }
    else {
      *offset = 0;
    }

    *left -= mem.size;
    ret->file_mems.push_back(mem);
    curr_size += mem.size;
  }

  ret->len = curr_size;
  return ret;
}

int piece_request_create(const torrent_info2 *torrent, unsigned index,
                         piece_request *out)
{
  std::cout << "enter piece_request_create" << std::endl;

  off_t offset = 0;
  unsigned files_vec_index = 0;
  skip_until_index(torrent, index, &offset, files_vec_index);
  std::cout << "offset: " << offset << std::endl;
  std::cout << "files_vec_index: " << files_vec_index << std::endl;

  out->piece_index = index;

  block_request *block = NULL;
  size_t left = torrent->piece_length;

  while ((block = next_block_request(torrent,
                                     &offset,
                                     &left,
                                     torrent->piece_length,
                                     files_vec_index))) {
    out->block_requests.push_back(block);
  }

  out->blocks_left = out->block_requests.size();

  return 0;
}
