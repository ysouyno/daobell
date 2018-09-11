#include "piece_request.h"
#include "peer_msg.h"

void skip_until_index(const torrent_info2 *torrent, unsigned index,
                      off_t *offset, unsigned &files_vec_index)
{
  size_t skip = torrent->piece_length * index;

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
      files_vec_index++;
    }
  }
}

block_request *next_block_request(const torrent_info2 *torrent,
                                  off_t *offset,
                                  size_t *left,
                                  size_t piece_len,
                                  unsigned &files_vec_index)
{
  // need to check files_vec_index here, otherwise it will lead
  // to an infinite loop
  if (0 == *left || files_vec_index >= torrent->files.size()) {
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

    file_mem *mem = new file_mem; // TODO: delete
    dnld_file_get_file_mem(file, mem);

    mem->mem = ((char *)mem->mem + *offset);
    mem->size -= *offset;

    if (mem->size > PEER_REQUEST_SIZE - curr_size) {
      mem->size = PEER_REQUEST_SIZE - curr_size;
      *offset += mem->size;
    }
    else {
      *offset = 0;
      files_vec_index++; // fix infinite loop
    }

    *left -= mem->size;
    ret->file_mems.push_back(mem);
    curr_size += mem->size;
  }

  ret->len = curr_size;
  return ret;
}

int piece_request_create(const torrent_info2 *torrent, unsigned index,
                         piece_request *out)
{
  off_t offset = 0;
  unsigned files_vec_index = 0;
  skip_until_index(torrent, index, &offset, files_vec_index);

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

block_request *piece_request_block_at(const piece_request *request,
                                      off_t offset)
{
  typedef std::list<block_request *>::const_iterator block_request_it;

  for (block_request_it it = request->block_requests.begin();
       it != request->block_requests.end();
       ++it) {
    if ((*it)->begin == offset) {
      return (*it);
    }
  }

  return NULL;
}
