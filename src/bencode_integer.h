#ifndef BENCODE_INTEGER_H
#define BENCODE_INTEGER_H

#include <iostream>
#include "bencode_value_base.h"

class bencode_integer : public bencode_value_base
{
public:
  explicit bencode_integer(long long value);

  void print_member();

  void crawl(bencode_crawler *p);

  long long get_value() const;

private:
  long long value_;
};

#endif /* BENCODE_INTEGER_H */
