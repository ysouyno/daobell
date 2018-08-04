#include "bt_downloader.h"
#include "bencode_parser.h"
#include "bencode_reader.h"
#include "bencode_value_safe_cast.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> > dict_map;
typedef std::shared_ptr<bencode_value_base> bencode_value_ptr;

struct torrent_info2
{
  std::string announce;
  std::string comment;
  std::string created_by;
  uint32_t creation_date;
  unsigned piece_length;
  bencode_list *files; // TODO: do not use this type
  char info_hash[20];
};

struct dnld_file
{
  pthread_mutex_t mutex;
  std::string path;
  unsigned size;
  unsigned char *data; // memory pointer
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

  std::cout << "create and open file at %s success" << file->path << std::endl;
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

    if (it->first == "pieces") {
      std::cout << "found pieces" << std::endl;
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
    }
  }

  if (!multifile) {
    std::string path = destdir;
    path += "/";
    path += file_name;

    std::cout << "path: " << path << std::endl;

    dnld_file *file = dnld_file_create_and_open(path, len);
    if (file) {
      // TODO: add file ptr to torrent_info2 list
      std::cout << "dnld_file_create_and_open success" << std::endl;
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
  memset(ret, 0, sizeof(torrent_info2));

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
      bencode_dictionary *value = down_cast<bencode_dictionary>(it->second.get());
      if (value) {
        populate_info_from_dict(value, destdir, ret);
      }
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
