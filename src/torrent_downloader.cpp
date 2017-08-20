#include "torrent_downloader.h"

torrent_downloader::torrent_downloader()
{
}

torrent_downloader::~torrent_downloader()
{
}

void *connect_tracker_thread(void *arg)
{
  log_t("connect_tracker_thread start\n");

  torrent_info *ti = (torrent_info *)arg;

  // connect tracker announce
  for (std::vector<std::string>::iterator it = ti->announce_list_.begin();
       it != ti->announce_list_.end(); ++it) {
    log_t("connecting %s\n", it->c_str());

    http_url_parser hup(it->c_str());

    if (hup.scheme_ == "https" ||
        hup.scheme_ == "udp") {
      log_w("not support https and udp for now\n");
      continue;
    }

    // convert a hex string to a real hex string ("d4" -> 0xd4)
    assert(!ti->info_hash_.empty());
    std::string hex;
    daobell_utils::hex_string_to_hex(ti->info_hash_, hex);

    // generate request string
    std::string request_str;
    request_str += "GET ";
    request_str += hup.path_;
    request_str += "?info_hash=";
    request_str += daobell_utils::percent_encode(hex);
    request_str += "&peer_id=";
    request_str += "-XXXXXX-%8D%22%8C%EE%A0%5C%FE%83%E6r%9B%BF";
    request_str += "&left=";

    // TODO: the total file size
    request_str += "304087040";
    request_str += " HTTP/1.1\r\nHost: ";
    request_str += hup.domain_;
    request_str += "\r\n\r\n";

    std::cout << "request string: " << request_str << std::endl;
  }

  return NULL;
}

int torrent_downloader::connect_tracker(pthread_t *tid, torrent_info *ti)
{
  if (0 == pthread_create(tid, NULL, connect_tracker_thread, (void *)ti)) {
    return 0;
  }

  return -1;
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

  get_info_hash(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));

  get_files(ti.get(), dynamic_cast<bencode_dictionary *>(sp_bvb.get()));
  std::cout << "get files: " << std::endl;
  for (std::vector<std::pair<std::string, long long> >::iterator it = ti->files_.begin();
       it != ti->files_.end(); ++it) {
    std::cout << "file path + name: " << it->first.c_str() << "\nlength: " << it->second << std::endl;
  }

  pthread_t tid = 0;

  connect_tracker(&tid, ti.get());

  if (0 != tid) {
    // waits for the thread specified by thread to terminate
    pthread_join(tid, NULL);
  }
}
