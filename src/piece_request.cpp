#include "piece_request.h"

void skip_until_index(const torrent_info2 *torrent, unsigned index,
                      off_t *offset)
{
  std::cout << "enter skip_until_index" << std::endl;

  size_t skip = torrent->piece_length * index;
  std::cout << "skip: " << skip << std::endl;

  for (std::vector<dnld_file *>::const_iterator it = torrent->files.begin();
       it != torrent->files.end() && skip > 0; ++it) {
    assert(*it);

    file_mem mem;
    dnld_file_get_file_mem(*it, &mem);

    // this is the last file to skip
    if (mem.size > skip) {
      *offset = skip;
      return;
    }
    else {
      skip -= mem.size;
    }
  }
}

int piece_request_create(const torrent_info2 *torrent, unsigned index,
                         piece_request *out)
{
  std::cout << "enter piece_request_create" << std::endl;

  off_t offset = 0;
  skip_until_index(torrent, index, &offset);
  std::cout << "offset: " << offset << std::endl;

  return 0;
}
