#ifndef BENCODE_DICTIONARY_H
#define BENCODE_DICTIONARY_H

#include <memory>
#include <map>
#include "bencode_value_base.h"

class bencode_dictionary : public bencode_value_base
{
public:
  void print_member();

  void crawl(bencode_crawler *p);

  void insert_to_dictionary(std::shared_ptr<bencode_value_base> key, std::shared_ptr<bencode_value_base> value);

  const std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> > &get_value() const;

private:
  std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> > value_;
};

#endif /* BENCODE_DICTIONARY_H */
