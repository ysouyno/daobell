#include "bencode_string.h"

bencode_string::bencode_string(const std::string &value) :
  value_(value)
{
}

void bencode_string::print_member()
{
  std::cout << value_ << std::endl;
}

void bencode_string::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

const std::string &bencode_string::get_value() const
{
  return value_;
}
