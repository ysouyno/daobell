#include "bt_downloader.h"
#include "bencode_parser.h"
#include "bencode_reader.h"
#include "bencode_value_safe_cast.h"
#include "bencode_encoder.h"
#include "sha1.h"
#include "tracker_announce.h"
#include "peer_info.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <list>

#define DEFAULT_MAX_PEERS 50
#define TRACKER_RETRY_INTERVAL 15

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

static const uint16_t g_port = 6889;

struct dnld_file
{
  pthread_mutex_t mutex;
  std::string path;
  unsigned size;
  unsigned char *data; // memory pointer
};

struct peer_conn
{
  pthread_t thread;
  peer_info peer;
};

struct torrent_info2
{
  std::string announce;
  std::string comment;
  std::string created_by;
  uint32_t creation_date;
  unsigned piece_length;
  std::vector<dnld_file *> files;
  char info_hash[20];
  pthread_t tracker_tid;
  unsigned max_peers;
  std::vector<std::string> pieces;

  struct
  {
    boost::dynamic_bitset<> pieces_state;
    std::list<peer_conn> peer_connections;
    unsigned pieces_left;
    unsigned uploaded;
    unsigned downloaded;
    bool completed;
  } sh;

  pthread_mutex_t sh_mutex;
};

struct tracker_arg
{
  struct torrent_info2 *torrent;
  uint16_t port;
};

dnld_file *dnld_file_create_and_open(const std::string &path, unsigned size)
{
  int fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);
  if (fd < 0) {
    perror("open");
    return NULL;
  }

  if (ftruncate(fd, size)) {
    perror("ftruncate");
    close(fd);
    return NULL;
  }

  struct stat file_stat = {0};
  if (fstat(fd, &file_stat)) {
    perror("fstat");
    close(fd);
    return NULL;
  }

  void *mem = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fd, 0);
  if (MAP_FAILED == mem) {
    perror("mmap");
    close(fd);
    return NULL;
  }

  dnld_file *file = new dnld_file;
  memset(file, 0, sizeof(dnld_file));

  pthread_mutex_init(&file->mutex, NULL);
  file->size = size;
  file->data = (unsigned char *)mem;
  file->path = path + ".incomplete";

  rename(path.c_str(), file->path.c_str());

  close(fd);

  std::cout << "create and open file success: " << file->path << std::endl;
  return file;
}

int dnld_file_close_and_free(dnld_file *file)
{
  int ret = 0;

  if (munmap(file->data, file->size)) {
    perror("munmap");
    ret = -1;
  }

  pthread_mutex_destroy(&file->mutex);
  delete(file);

  return ret;
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

void create_local_peer_id(char outbuff[20])
{
  int offset = 0;

  const char *id = "ys";

  memset(outbuff, 0, 20);
  offset += snprintf(outbuff, 20, "-%.*s%02u%02u-", 2, id, 1, 0);

  for (unsigned i = 0; i < 12 / (sizeof(int32_t)); i++) {
    int32_t r = rand();
    memcpy(outbuff + offset, &r, sizeof(r));
    offset += sizeof(r);
  }
}

std::shared_ptr<tracker_announce_req> create_tracker_request(const void *arg)
{
  std::cout << "enter create_tracker_request" << std::endl;

  char local_peer_id[20] = {0};
  create_local_peer_id(local_peer_id);

  const tracker_arg *targ = (tracker_arg *)arg;
  std::shared_ptr<tracker_announce_req> ret =
    std::make_shared<tracker_announce_req>();
  if (ret) {
    ret->has = 0;
    memcpy(ret->info_hash, targ->torrent->info_hash, sizeof(ret->info_hash));
    memcpy(ret->peer_id, local_peer_id, sizeof(ret->peer_id));
    ret->port = targ->port;
    ret->compact = true;
    SET_HAS(ret, REQUEST_HAS_COMPACT);

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    unsigned num_conns = targ->torrent->sh.peer_connections.size();
    pthread_mutex_unlock(&targ->torrent->sh_mutex);

    ret->numwant = targ->torrent->max_peers - num_conns;
    SET_HAS(ret, REQUEST_HAS_NUMWANT);

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    ret->uploaded = targ->torrent->sh.uploaded;
    ret->downloaded = targ->torrent->sh.downloaded;
    ret->left = 0; // TODO
    pthread_mutex_unlock(&targ->torrent->sh_mutex);
  }

  return ret;
}

static void *periodic_announce(void *arg)
{
  std::cout << "periodic_announce" << std::endl;

  const tracker_arg *targ = (tracker_arg *)arg;
  bool completed = false;
  unsigned interval = 0;

  pthread_mutex_lock(&targ->torrent->sh_mutex);
  completed = targ->torrent->sh.completed;
  pthread_mutex_unlock(&targ->torrent->sh_mutex);

  bool started = false;
  int i = 0;
  while (true && i++ < 1) {
    std::cout << "while (true) start" << std::endl;
    std::shared_ptr<tracker_announce_req> req = create_tracker_request(targ);


    if (!started) {
      req->event = TORRENT_EVENT_STARTED;
      SET_HAS(req, REQUEST_HAS_EVENT);
      started = true;
    }

    pthread_mutex_lock(&targ->torrent->sh_mutex);
    bool current_completed = targ->torrent->sh.completed;
    pthread_mutex_unlock(&targ->torrent->sh_mutex);

    if (false == completed && true == current_completed) {
      req->event = TORRENT_EVENT_COMPLETED;
      SET_HAS(req, REQUEST_HAS_EVENT);
    }

    completed = current_completed;

    std::shared_ptr<tracker_announce_resp> resp =
      tracker_announce(targ->torrent->announce, req.get());

    if (resp) {
      print_tracker_announce_resp(resp.get());
      interval = resp->interval;
      std::cout << "re-announcing to tracker again in " << interval
                << " seconds" << std::endl;
    }
    else {
      interval = TRACKER_RETRY_INTERVAL;
      std::cout << "retrying announcing to tracker in " << interval
                << " seconds" << std::endl;
    }

    // sleep(interval);
  }

  return NULL;
}

int tracker_connection_create(pthread_t *tid, tracker_arg *arg)
{
  if (pthread_create(tid, NULL, periodic_announce, (void *)arg)) {
    return -1;
  }

  return 0;
}

void bt_download(const std::string &metafile, const std::string &destdir)
{
  std::ifstream file(metafile, std::ios::binary);
  bencode_parser bp(file);

  std::shared_ptr<bencode_value_base> sp_bvb = bp.get_value();
  bencode_reader br(sp_bvb);

  torrent_info2 *torrent = torrent_init(sp_bvb, destdir);

  std::shared_ptr<tracker_arg> arg = std::make_shared<tracker_arg>();
  arg->torrent = torrent;
  arg->port = g_port;
  if (tracker_connection_create(&torrent->tracker_tid, arg.get())) {
    std::cout << "tracker_connection_create failed" << std::endl;
  }

  pthread_join(torrent->tracker_tid, NULL);

  delete torrent;
}
