#include "torrent_downloader.h"

torrent_downloader::torrent_downloader()
{
}

torrent_downloader::~torrent_downloader()
{
}

void torrent_downloader::download_it(const std::string &torrent_file, const std::string &dest_dir)
{
  std::ifstream file(torrent_file.c_str(), std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  auto ti = std::make_shared<torrent_info>();
  get_announce(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  for (std::vector<std::string>::iterator it = ti->announce_list_.begin();
       it != ti->announce_list_.end(); ++it) {
    std::cout << "announce: " << it->c_str() << std::endl;
  }
}
