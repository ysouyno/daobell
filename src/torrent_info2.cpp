#include "torrent_info2.h"
#include <iostream>
#include <assert.h>

int create_pieces_vector(const bencode_string *pieces, torrent_info2 *torrent)
{
  assert(pieces->get_value().length() % 20 == 0);

  for (unsigned i = 0; i < pieces->get_value().length(); i += 20) {
    torrent->pieces.push_back(pieces->get_value().substr(i, 20));
  }

  assert(torrent->pieces.at(torrent->pieces.size() - 1).size() == 20);

  return 0;
}

int populate_files_from_list(bencode_list *files, const std::string &destdir,
                             const std::string &name, torrent_info2 *torrent)
{
  int ret = 0;

  std::string path = destdir;
  path += "/";
  path += name;

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
        if (it->first == "length") {
          bencode_integer *dict_key =
            down_cast<bencode_integer>(it->second.get());
          len = dict_key->get_value();
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

    dnld_file *file = dnld_file_create_and_open(path, len);
    if (file) {
      torrent->files.push_back(file);

      // TODO: don't forget close and free somewhere
      // dnld_file_close_and_free(file); // fix recv: Bad address
    }
  }

  return ret;
}

int populate_info_from_dict(bencode_dictionary *info, const std::string &destdir,
                            torrent_info2 *torrent)
{
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
      }
    }

    if (it->first == "length") {
      bencode_integer *value = down_cast<bencode_integer>(it->second.get());
      if (value) {
        len = value->get_value();
      }
    }

    if (it->first == "files") {
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

    dnld_file *file = dnld_file_create_and_open(path, len);
    if (file) {
      std::cout << "dnld_file_create_and_open success" << std::endl;
      torrent->files.push_back(file);

      // TODO: don't forget close and free somewhere
      // dnld_file_close_and_free(file); // fix recv: Bad address
    }
    else {
      ret = -1;
    }
  }

  return ret;
}

torrent_info2 *torrent_init(bencode_value_ptr meta, const std::string &destdir)
{
  torrent_info2 *ret = new torrent_info2;
  // memset(ret, 0, sizeof(torrent_info2)); // fix segmentation fault

  bencode_dictionary *dict = down_cast<bencode_dictionary>(meta.get());
  dict_map dictmap = dict->get_value();

  for (dict_map::iterator it = dictmap.begin(); it != dictmap.end(); ++it) {
    if (it->first == "announce") {
      bencode_string *value = down_cast<bencode_string>(it->second.get());
      if (value) {
        ret->announce = value->get_value();
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
      }
    }

    if (it->first == "info") {
      // compute info_hash
      bencode_value_base *info_dict = it->second.get();

      bencode_encoder be(info_dict);
      be.encode();
      std::string encoded_info_dict = be.get_value();

      char sha1_result[20] = {0};
      sha1_compute(encoded_info_dict.c_str(), encoded_info_dict.size(),
                   sha1_result);

      memcpy(ret->info_hash, sha1_result, 20);

      bencode_dictionary *value =
        down_cast<bencode_dictionary>(it->second.get());
      if (value) {
        populate_info_from_dict(value, destdir, ret);
      }
    }
  }

  pthread_mutex_init(&ret->sh_mutex, NULL);
  ret->max_peers = DEFAULT_MAX_PEERS;
  ret->sh.pieces_state.resize(ret->pieces.size());
  ret->sh.pieces_state.shrink_to_fit();
  ret->sh.pieces_left = ret->pieces.size();
  ret->sh.uploaded = 0;
  ret->sh.downloaded = 0;
  ret->sh.completed = false;

  // TODO: need to free memory somewhere
  // delete ret;

  return ret;
}

int torrent_make_bitfield(const torrent_info2 *torrent,
                          boost::dynamic_bitset<> *out)
{
  std::cout << "enter torrent_make_bitfield" << std::endl;
  assert(torrent);

  unsigned num_pieces = torrent->pieces.size();

  out->resize(num_pieces, false);

  for (unsigned i = 0; i < num_pieces; ++i) {
    if (torrent->sh.pieces_state.at(i) == PIECE_STATE_HAVE) {
      (*out)[i] = 1;
    }
  }

  return 0;
}

int torrent_next_request(torrent_info2 *torrent,
                         boost::dynamic_bitset<> *peer_have,
                         unsigned *out)
{
  unsigned request = 0, not_request = 0;
  bool has_request = false, has_not_request = false;

  assert(torrent->pieces.size() == torrent->sh.pieces_state.size());
  assert(torrent->pieces.size() == (*peer_have).size());

  pthread_mutex_lock(&torrent->sh_mutex);

  for (unsigned i = 0; i < torrent->pieces.size(); ++i) {
    if (torrent->sh.pieces_state[i] == PIECE_STATE_REQUESTED &&
        (*peer_have)[i] == 1) {
      request = i;
      has_request = true;
    }

    if (torrent->sh.pieces_state[i] == PIECE_STATE_NOT_REQUESTED &&
        (*peer_have)[i] == 1) {
      not_request = i;
      has_not_request = true;
      break;
    }
  }

  if (!has_request && !has_not_request) {
    pthread_mutex_unlock(&torrent->sh_mutex);
    return -1;
  }

  unsigned ret = has_not_request ? not_request : request;
  torrent->sh.pieces_state[ret] = PIECE_STATE_REQUESTED;

  pthread_mutex_unlock(&torrent->sh_mutex);

  std::cout << "requesting piece: " << ret << std::endl;
  *out = ret;

  return 0;
}

bool torrent_sha1_verify(const torrent_info2 *torrent, unsigned index)
{
  std::cout << "enter torrent_sha1_verify" << std::endl;

  return true;
}

int torrent_complete(torrent_info2 *torrent)
{
  pthread_mutex_lock(&torrent->sh_mutex);
  torrent->sh.completed = true;
  // TODO torrent state
  pthread_mutex_unlock(&torrent->sh_mutex);

  for (std::vector<dnld_file *>::iterator it = torrent->files.begin();
       it != torrent->files.end();
       ++it) {
    dnld_file_complete(*it);
  }

  std::cout << "torrent completed" << std::endl;
  return 0;
}
