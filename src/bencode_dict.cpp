#include "bencode_dict.h"

void bencode_dict::print_member()
{
  for (std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = value_.begin();
       it != value_.end(); ++it) {
    std::cout << it->first << ": ";
    it->second.get()->print_member();
    std::cout << std::endl;
  }
}

void bencode_dict::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

void bencode_dict::insert_to_dictionary(std::string key, std::shared_ptr<bencode_value_base> value)
{
  value_.insert(std::make_pair(key, value));
}

const std::multimap<std::string, std::shared_ptr<bencode_value_base> > &bencode_dict::get_value() const
{
  return value_;
}

bencode_value_base *bencode_dict::get(const std::string &key)
{
  std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = value_.find(key);
  if (std::end(value_) == it) {
    return NULL;
  }
  else {
    return it->second.get();
  }
}

std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator bencode_dict::begin()
{
  return value_.begin();
}

std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator bencode_dict::end()
{
  return value_.end();
}
