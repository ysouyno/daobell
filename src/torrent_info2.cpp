#include "torrent_info2.h"
#include <iostream>
#include <assert.h>

int create_pieces_vector(const bencode_string *pieces, torrent_info2 *torrent)
{
  std::cout << "enter create_pieces_vector" << std::endl;
  assert(pieces->get_value().length() % 20 == 0);

  std::cout << "pieces's length: " << pieces->get_value().length() << std::endl;

  for (unsigned i = 0; i < pieces->get_value().length(); i += 20) {
    torrent->pieces.push_back(pieces->get_value().substr(i, 20));
  }

  assert(torrent->pieces.at(torrent->pieces.size() - 1).size() == 20);

  return 0;
}

int populate_files_from_list(bencode_list *files, const std::string &destdir,
                             const std::string &name, torrent_info2 *torrent)
{
  std::cout << "enter populate_files_from_list" << std::endl;
  int ret = 0;

  std::string path = destdir;
  path += "/";
  path += name;

  std::cout << "creating directory: " << path << std::endl;
  mkdir(path.c_str(), 0777);

  for (auto &f : *files) {
    bencode_dictionary *value = down_cast<bencode_dictionary>(f.get());
    std::string path = destdir;
    path += "/";
    path += name;
    path += "/";
    unsigned len = 0;

    if (value) {
      dict_map dict = value->get_value();
      for (dict_map::iterator it = dict.begin(); it != dict.end(); ++it) {
        std::cout << "it->first: " << it->first << std::endl;
        if (it->first == "length") {
          bencode_integer *dict_key =
            down_cast<bencode_integer>(it->second.get());
          len = dict_key->get_value();
          std::cout << "    lenght: " << len << std::endl;
        }

        if (it->first == "path") {
          int i = 0;
          bencode_list *path_list = down_cast<bencode_list>(it->second.get());
          for (auto &p : *path_list) {
            bencode_string *path_str = down_cast<bencode_string>(p.get());
            path += path_str->get_value();
            if (i < (int)(path_list->get_value().size()) - 1) {
              mkdir(path.c_str(), 0777);
              path += "/";
            }
            ++i;
          }
        }
      }
    }

    std::cout << "    path: " << path << std::endl;
    dnld_file *file = dnld_file_create_and_open(path, len);
    if (file) {
      std::cout << "dnld_file_create_and_open success" << std::endl;
      torrent->files.push_back(file);

      // TODO: don't forget close and free somewhere
      dnld_file_close_and_free(file);
    }
  }

  return ret;
}

int populate_info_from_dict(bencode_dictionary *info, const std::string &destdir,
                            torrent_info2 *torrent)
{
  std::cout << "enter populate_info_from_dict" << std::endl;
  int ret = 0;
  bool multifile = false;
  std::string file_name;
  unsigned len = 0;

  dict_map info_dict = info->get_value();

  for (dict_map::iterator it = info_dict.begin(); it != info_dict.end(); ++it) {
    if (it->first == "name") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        file_name = value->get_value();
        std::cout << "name: " << file_name << std::endl;
      }
    }
  }

  for (dict_map::iterator it = info_dict.begin(); it != info_dict.end(); ++it) {
    if (it->first == "pieces") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        create_pieces_vector(value, torrent);
      }
    }

    if (it->first == "piece length") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        torrent->piece_length = value->get_value();
        std::cout << "piece length: " << torrent->piece_length << std::endl;
      }
    }

    if (it->first == "length") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        len = value->get_value();
        std::cout << "length: " << len << std::endl;
      }
    }

    if (it->first == "files") {
      std::cout << "found files" << std::endl;
      multifile = true;

      // files: a list of dictionaries, one for each file. Each dictionary in
      // this list contains the following keys:
      //
      //   length: length of the file in bytes (integer)
      //   md5sum: (optional) a 32-character hexadecimal string corresponding
      //           to the MD5 sum of the file.
      //   path  : a list containing one or more string elements that together
      //           represent the path and filename.
      bencode_list *value = down_cast<bencode_list>(it->second.get());
      if (value) {
        populate_files_from_list(value, destdir, file_name, torrent);
      }
    }
  }

  // single-file mode
  if (!multifile) {
    std::string path = destdir;
    path += "/";
    path += file_name;

    std::cout << "path: " << path << std::endl;

    dnld_file *file = dnld_file_create_and_open(path, len);
    if (file) {
      std::cout << "dnld_file_create_and_open success" << std::endl;
      torrent->files.push_back(file);

      // TODO: don't forget close and free somewhere
      dnld_file_close_and_free(file);
    }
    else {
      ret = -1;
    }
  }

  return ret;
}

torrent_info2 *torrent_init(bencode_value_ptr meta, const std::string &destdir)
{
  std::cout << "enter torrent_init" << std::endl;

  torrent_info2 *ret = new torrent_info2;
  // memset(ret, 0, sizeof(torrent_info2)); // fix segmentation fault

  bencode_dictionary *dict = down_cast<bencode_dictionary>(meta.get());
  dict_map dictmap = dict->get_value();

  for (dict_map::iterator it = dictmap.begin(); it != dictmap.end(); ++it) {
    if (it->first == "announce") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        ret->announce = value->get_value();
        std::cout << "announce: " << ret->announce << std::endl;
      }
    }

    if (it->first == "comment") {
      std::cout << "found comment" << std::endl;
    }

    if (it->first == "created by") {
      std::cout << "found create by" << std::endl;
    }

    if (it->first == "creation date") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        ret->creation_date = value->get_value();
        std::cout << "creation date: " << ret->creation_date << std::endl;
      }
    }

    if (it->first == "info") {
      std::cout << "found info" << std::endl;
      // compute info_hash
      bencode_value_base *info_dict = it->second.get();

      bencode_encoder be(info_dict);
      be.encode();
      std::string encoded_info_dict = be.get_value();

      char sha1_result[20] = {0};
      sha1_compute(encoded_info_dict.c_str(), encoded_info_dict.size(), sha1_result);

      for (int i = 0; i < 20; ++i) {
        printf("%02x", (unsigned char)sha1_result[i]);
      }
      printf("\n");
      memcpy(ret->info_hash, sha1_result, 20);

      bencode_dictionary *value = down_cast<bencode_dictionary>(it->second.get());
      if (value) {
        populate_info_from_dict(value, destdir, ret);
      }
    }
  }

  pthread_mutex_init(&ret->sh_mutex, NULL);
  ret->max_peers = DEFAULT_MAX_PEERS;
  ret->sh.pieces_state.resize(ret->pieces.size(), false);
  std::cout << "torrent::sh::pieces_state: " << ret->sh.pieces_state << std::endl;
  ret->sh.pieces_left = ret->pieces.size();
  std::cout << "torrent::sh::pieces_left: " << ret->pieces.size() << std::endl;
  ret->sh.uploaded = 0;
  ret->sh.downloaded = 0;
  ret->sh.completed = false;

  // TODO: need to free memory somewhere
  // delete ret;

  return ret;
}
