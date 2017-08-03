#ifndef BENCODE_DICTIONARY_H
#define BENCODE_DICTIONARY_H

#include <iostream>
#include <memory>
#include <map>
#include <string>
#include "bencode_value_base.h"

class bencode_dictionary : public bencode_value_base
{
public:
  void print_member();

  void crawl(bencode_crawler *p);

  void insert_to_dictionary(std::string key, std::shared_ptr<bencode_value_base> value);

  const std::multimap<std::string, std::shared_ptr<bencode_value_base> > &get_value() const;

private:
  std::multimap<std::string, std::shared_ptr<bencode_value_base> > value_;
};

#endif /* BENCODE_DICTIONARY_H */
