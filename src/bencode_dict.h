#ifndef BENCODE_DICT_H
#define BENCODE_DICT_H

#include "bencode_value_base.h"
#include <iostream>
#include <memory>
#include <map>
#include <string>

class bencode_dict : public bencode_value_base
{
 public:
  typedef std::multimap<std::string, std::shared_ptr<bencode_value_base> >
    value_type;

  void print_member();

  void crawl(bencode_crawler *p);

  void insert_to_dict(std::string key,
                      std::shared_ptr<bencode_value_base> value);

  const value_type &get_value() const;

  bencode_value_base *get(const std::string &key);

  value_type::iterator begin();

  value_type::iterator end();

 private:
  value_type value_;
};

#endif /* BENCODE_DICT_H */
