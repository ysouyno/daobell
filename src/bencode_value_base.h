#ifndef BENCODE_VALUE_BASE_H
#define BENCODE_VALUE_BASE_H

#include "bencode_crawler.h"

class bencode_value_base
{
public:
  virtual void print_member() = 0;
  virtual void crawl(bencode_crawler *p) = 0;
};

#endif /* BENCODE_VALUE_BASE_H */
