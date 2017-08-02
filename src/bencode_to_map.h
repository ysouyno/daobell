#ifndef BENCODE_TO_MAP_H
#define BENCODE_TO_MAP_H

#include <memory>
#include <map>
#include <string.h>
#include "bencode_crawler.h"
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dictionary.h"

class bencode_to_map : public bencode_crawler
{
public:
  explicit bencode_to_map(std::shared_ptr<bencode_value_base> sp_bencode_parser);

  virtual void crawl(bencode_string *p);

  virtual void crawl(bencode_integer *p);

  virtual void crawl(bencode_list *p);

  virtual void crawl(bencode_dictionary *p);

  const std::string &get_list_name() const;

  void print_all();

  void crawl_all();

  void print_multimap();

private:
  std::string crawl_str_;
  std::shared_ptr<bencode_value_base> sp_bencode_parser_;
  std::multimap<std::string, std::string> multimap_dictionary_;
  std::string list_name_;
};

#endif /* BENCODE_TO_MAP_H */
