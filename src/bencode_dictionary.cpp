#include "bencode_dictionary.h"

void bencode_dictionary::print_member()
{
  for (std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> >::iterator it = value_.begin();
       it != value_.end(); ++it) {
    it->first.get()->print_member();
    it->second.get()->print_member();
  }
}

void bencode_dictionary::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

void bencode_dictionary::insert_to_dictionary(std::shared_ptr<bencode_value_base> key, std::shared_ptr<bencode_value_base> value)
{
  value_.insert(std::make_pair(key, value));
}

const std::multimap<std::shared_ptr<bencode_value_base>, std::shared_ptr<bencode_value_base> > &bencode_dictionary::get_value() const
{
  return value_;
}
