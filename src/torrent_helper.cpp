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

  ti->piece_length_ = bi->get_value();

  return bi->get_value();
}

long long get_creation_date(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb = root->get("creation date");
  assert(bvb != NULL);

  bencode_integer *bi = dynamic_cast<bencode_integer *>(bvb);
  assert(bi != NULL);

  ti->creation_date_ = bi->get_value();

  return bi->get_value();
}

void get_info_hash(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb = root->get("info");
  assert(bvb != NULL);

  bencode_encoder be(bvb);
  be.encode();
  be.print_result();

  std::string encoded_info_dict = be.get_value();
  char sha1_result[20] = {0};
  sha1_compute(encoded_info_dict.c_str(), encoded_info_dict.size(), sha1_result);
  for (size_t i = 0; i < 20; i++) {
    printf("%02X", (unsigned char)sha1_result[i]);
  }
  printf("\n");
}
