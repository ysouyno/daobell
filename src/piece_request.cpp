#include "piece_request.h"
#include "peer_msg.h"

void skip_until_index(const torrent_info *torrent, unsigned index,
                      off_t *offset, unsigned &files_vec_index)
{
  size_t skip = torrent->piece_length * index;

  for (unsigned i = 0; i < torrent->files.size() && skip > 0; ++i) {
    assert(torrent->files[i]);

    std::shared_ptr<file_mem> mem = std::make_shared<file_mem>();
    mem = torrent->files[i]->get_file_mem();

    // this is the last file to skip
    if (mem->size > skip) {
      *offset = skip;
      files_vec_index = i;
      return;
    }
    else {
      skip -= mem->size;
      files_vec_index++;
    }
  }
}

std::shared_ptr<block_request> next_block_request(const torrent_info *torrent,
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

  std::shared_ptr<block_request> ret = std::make_shared<block_request>();
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
    std::shared_ptr<dnld_file> file = torrent->files[i];
    assert(file);

    std::shared_ptr<file_mem> mem = std::make_shared<file_mem>();
    mem->mem = file->get_file_mem()->mem;
    mem->size = file->get_file_mem()->size;

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

int piece_request_create(const torrent_info *torrent, unsigned index,
                         piece_request *out)
{
  off_t offset = 0;
  unsigned files_vec_index = 0;
  skip_until_index(torrent, index, &offset, files_vec_index);

  out->piece_index = index;

  std::shared_ptr<block_request> block = NULL;
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
  typedef std::list<std::shared_ptr<block_request> > block_req_list;

  for (block_req_list::const_iterator it = request->block_requests.begin();
       it != request->block_requests.end();
       ++it) {
    if ((*it)->begin == offset) {
      return (*it).get();
    }
  }

  return NULL;
}
