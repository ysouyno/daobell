#include "bencode_dictionary.h"

void bencode_dictionary::print_member()
{
  for (std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = value_.begin();
       it != value_.end(); ++it) {
    std::cout << it->first << ": ";
    it->second.get()->print_member();
    std::cout << std::endl;
  }
}

void bencode_dictionary::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

void bencode_dictionary::insert_to_dictionary(std::string key, std::shared_ptr<bencode_value_base> value)
{
  value_.insert(std::make_pair(key, value));
}

const std::multimap<std::string, std::shared_ptr<bencode_value_base> > &bencode_dictionary::get_value() const
{
  return value_;
}

bencode_value_base *bencode_dictionary::get(const std::string &key)
{
  std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator it = value_.find(key);
  if (std::end(value_) == it) {
    return NULL;
  }
  else {
    return it->second.get();
  }
}

std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator bencode_dictionary::begin()
{
  return value_.begin();
}

std::multimap<std::string, std::shared_ptr<bencode_value_base> >::iterator bencode_dictionary::end()
{
  return value_.end();
}