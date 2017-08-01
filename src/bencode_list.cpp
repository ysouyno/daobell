#include "bencode_list.h"

void bencode_list::print_member()
{
  for (std::vector<std::shared_ptr<bencode_value_base> >::iterator it = value_.begin();
       it != value_.end(); ++it) {
    (*it)->print_member();
  }
}

void bencode_list::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

void bencode_list::insert_to_list(std::shared_ptr<bencode_value_base> value)
{
  value_.push_back(value);
}

const std::vector<std::shared_ptr<bencode_value_base> > &bencode_list::get_value() const
{
  return value_;
}
