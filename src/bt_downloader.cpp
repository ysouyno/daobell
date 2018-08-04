#include "bt_downloader.h"
#include "bencode_parser.h"
#include "bencode_reader.h"
#include "bencode_value_safe_cast.h"
#include <string.h>

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

struct torrent_info2
{
  std::string announce;
  std::string comment;
  std::string created_by;
  uint32_t creation_date;
  bencode_list *files;
  char info_hash[20];
};

torrent_info2 *torrent_init(bencode_value_ptr meta, const std::string &destdir)
{
  std::cout << "enter torrent_init" << std::endl;

  torrent_info2 *ret = new torrent_info2;
  memset(ret, 0, sizeof(torrent_info2));

  bencode_dictionary *dict = down_cast<bencode_dictionary>(meta.get());
  dict_map dictmap = dict->get_value();

  for (dict_map::iterator it = dictmap.begin(); it != dictmap.end(); ++it) {
    if (it->first == "announce") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      ret->announce = value->get_value();
      std::cout << "announce: " << ret->announce << std::endl;
    }

    if (it->first == "comment") {
      std::cout << "found comment" << std::endl;
    }

    if (it->first == "created by") {
      std::cout << "found create by" << std::endl;
    }

    if (it->first == "creation date") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      ret->creation_date = value->get_value();
      std::cout << "creation date: " << ret->creation_date << std::endl;
    }
  }

  return ret;
}

void bt_download(const std::string &metafile, const std::string &destdir)
{
  std::ifstream file(metafile, std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_reader br(sp_bvb);

  torrent_init(sp_bvb, destdir);
}
