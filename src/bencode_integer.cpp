#include "bencode_integer.h"

bencode_integer::bencode_integer(long long value) :
  value_(value)
{
}

void bencode_integer::print_member()
{
  std::cout << value_ << std::endl;
}

void bencode_integer::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

long long bencode_integer::get_value() const
{
  return value_;
}
