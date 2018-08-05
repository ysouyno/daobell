#include "bencode_list.h"

void bencode_list::print_member()
{
  for (value_type::iterator it = value_.begin(); it != value_.end(); ++it) {
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

const bencode_list::value_type &bencode_list::get_value() const
{
  return value_;
}

bencode_list::value_type::iterator bencode_list::begin()
{
  return value_.begin();
}

bencode_list::value_type::iterator bencode_list::end()
{
  return value_.end();
}
