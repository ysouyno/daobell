#include "bencode_int.h"

bencode_int::bencode_int(long long value) :
  value_(value)
{
}

void bencode_int::print_member()
{
  std::cout << value_ << std::endl;
}

void bencode_int::crawl(bencode_crawler *p)
{
  p->crawl(this);
}

long long bencode_int::get_value() const
{
  return value_;
}
