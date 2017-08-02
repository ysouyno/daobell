#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include <fstream>
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dictionary.h"

class bencode_parser
{
public:
  bencode_parser() = default;
  ~bencode_parser() = default;

  explicit bencode_parser(std::ifstream &ifs);

  std::shared_ptr<bencode_value_base> parse(std::ifstream &ifs);

  void print_all();

  std::shared_ptr<bencode_value_base> get_value();

private:
  std::shared_ptr<bencode_value_base> value_;
};

#endif /* BENCODE_PARSER_H */
