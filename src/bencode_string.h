#ifndef BENCODE_STRING_H
#define BENCODE_STRING_H

#include <iostream>
#include <string>
#include "bencode_value_base.h"

class bencode_string : public bencode_value_base
{
public:
  explicit bencode_string(const std::string &value);

  void print_member();

  void crawl(bencode_crawler *p);

  const std::string &get_value() const;

private:
  std::string value_;
};

#endif /* BENCODE_STRING_H */
