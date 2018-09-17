#ifndef BENCODE_INT_H
#define BENCODE_INT_H

#include "bencode_value_base.h"
#include <iostream>

class bencode_int : public bencode_value_base
{
 public:
  explicit bencode_int(long long value);

  void print_member();

  void crawl(bencode_crawler *p);

  long long get_value() const;

 private:
  long long value_;
};

#endif /* BENCODE_INT_H */
