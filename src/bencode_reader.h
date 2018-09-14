#ifndef BENCODE_READER_H
#define BENCODE_READER_H

#include <string>
#include <memory>
#include <map>
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_dict.h"

class bencode_reader
{
public:
  bencode_reader() = default;
  ~bencode_reader() = default;

  explicit bencode_reader(std::shared_ptr<bencode_value_base> sp_bvb);

  const std::string &get_announce();

private:
  std::shared_ptr<bencode_value_base> sp_bvb_;
};

#endif /* BENCODE_READER_H */
