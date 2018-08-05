#ifndef BENCODE_LIST_H
#define BENCODE_LIST_H

#include <vector>
#include <memory>
#include "bencode_value_base.h"

class bencode_list : public bencode_value_base
{
public:
  typedef std::vector<std::shared_ptr<bencode_value_base> > value_type;

  void print_member();

  void crawl(bencode_crawler *p);

  void insert_to_list(std::shared_ptr<bencode_value_base> value);

  const value_type &get_value() const;

  value_type::iterator begin();

  value_type::iterator end();

private:
  value_type value_;
};

#endif /* BENCODE_LIST_H */
