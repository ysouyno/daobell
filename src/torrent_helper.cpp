#include "torrent_helper.h"

void get_announce(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb_announce = root->get("announce-list");
  if (NULL != bvb_announce) {
    bencode_list *announce_list = dynamic_cast<bencode_list *>(bvb_announce);
    assert(announce_list != NULL);

    for (auto &list : *announce_list) {
      bencode_list *bl = dynamic_cast<bencode_list *>(list.get());
      assert(bl != NULL);

      for (auto &elem : *bl) {
        bencode_string *bs = dynamic_cast<bencode_string *>(elem.get());
        assert(bs != NULL);
        std::cout << bs->get_value() << std::endl;
        ti->announce_list_.push_back(bs->get_value());
      }
    }
  }
  else {
    // just have one main announce url
    bvb_announce = root->get("announce");
    bencode_string *announce = dynamic_cast<bencode_string *>(bvb_announce);
    assert(announce != NULL);
    ti->announce_list_.push_back(announce->get_value());
  }
}

long long get_piece_length(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb = root->get("info");
  assert(bvb != NULL);

  bencode_dictionary *info_dict = dynamic_cast<bencode_dictionary *>(bvb);
  assert(info_dict != NULL);

  bencode_integer *bi = dynamic_cast<bencode_integer *>(info_dict->get("piece length"));
  assert(bi != NULL);

  return bi->get_value();
}

long long get_creation_date(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb = root->get("creation date");
  assert(bvb != NULL);

  bencode_integer *bi = dynamic_cast<bencode_integer *>(bvb);
  assert(bi != NULL);

  return bi->get_value();
}
