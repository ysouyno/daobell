#ifndef BENCODE_LIST_H
#define BENCODE_LIST_H

#include <vector>
#include <memory>
#include "bencode_value_base.h"

class bencode_list : public bencode_value_base
{
public:
  void print_member();

  void crawl(bencode_crawler *p);

  void insert_to_list(std::shared_ptr<bencode_value_base> value);

  const std::vector<std::shared_ptr<bencode_value_base> > &get_value() const;

private:
  std::vector<std::shared_ptr<bencode_value_base> > value_;
};

#endif /* BENCODE_LIST_H */
