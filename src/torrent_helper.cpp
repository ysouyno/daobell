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

  char temp[8] = {0};
  ti->info_hash_.clear();
  for (size_t i = 0; i < 20; i++) {
    snprintf(temp, sizeof(temp), "%02X", (unsigned char)sha1_result[i]);
    ti->info_hash_.append(temp, 2);
  }
}

void get_files_and_size(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb_info = root->get("info");
  assert(bvb_info != NULL);

  ti->files_size_ = 0;

  bencode_value_base *bvb_files = dynamic_cast<bencode_dictionary *>(bvb_info)->get("files");
  if (bvb_files != NULL) {
    std::cout << "multi files mode" << std::endl;

    bencode_value_base *bvb_name = dynamic_cast<bencode_dictionary *>(bvb_info)->get("name");
    assert(bvb_name != NULL);

    bencode_string *root_dir = dynamic_cast<bencode_string *>(bvb_name);
    std::string dir_name = root_dir->get_value();

    bencode_list *files_list = dynamic_cast<bencode_list *>(bvb_files);
    assert(files_list != NULL);

    for (auto &file : *files_list) {
      bencode_dictionary *file_dict = dynamic_cast<bencode_dictionary *>(file.get());
      assert(file_dict != NULL);

      bencode_integer *length = dynamic_cast<bencode_integer *>(file_dict->get("length"));
      assert(length != NULL);

      ti->files_size_ += length->get_value();

      bencode_list *file_path_list = dynamic_cast<bencode_list *>(file_dict->get("path"));
      assert(file_path_list != NULL);

      std::string file_name = "";
      for (auto &path : *file_path_list) {
        bencode_string *file_path = dynamic_cast<bencode_string *>(path.get());
        // add the root directory
        file_name += dir_name;
        file_name += '/';
        file_name += file_path->get_value();
        file_name += '/';
      }

      // remove the last '/'
      file_name = file_name.substr(0, file_name.size() - 1);

      auto file_info = std::make_pair(file_name, length->get_value());
      ti->files_.push_back(file_info);
    }
  }
  else {
    std::cout << "single file mode" << std::endl;

    bencode_value_base *bvb_length = dynamic_cast<bencode_dictionary *>(bvb_info)->get("length");
    assert(bvb_length != NULL);

    bencode_integer *length = dynamic_cast<bencode_integer *>(bvb_length);

    ti->files_size_ = length->get_value();

    bencode_value_base *bvb_name = dynamic_cast<bencode_dictionary *>(bvb_info)->get("name");
    assert(bvb_name != NULL);

    bencode_string *name = dynamic_cast<bencode_string *>(bvb_name);

    auto file_info = std::make_pair(name->get_value(), length->get_value());
    ti->files_.push_back(file_info);
  }
}

void get_peers(torrent_info *ti, bencode_dictionary *root)
{
  bencode_value_base *bvb_peers = root->get("peers");
  assert(bvb_peers != NULL);

  bencode_peers_crawler bpc(ti, bvb_peers);
  bpc.parse();
}

void get_peer_id(torrent_info *ti)
{
  ti->peer_id_ = ti->info_hash_;
}
