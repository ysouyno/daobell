#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include <fstream>
#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dict.h"

class bencode_parser
{
public:
  bencode_parser() = default;
  ~bencode_parser() = default;

  explicit bencode_parser(std::ifstream &ifs);

  explicit bencode_parser(const char *p);

  std::shared_ptr<bencode_value_base> parse(std::ifstream &ifs);

  std::shared_ptr<bencode_value_base> parse(const char *p, const char **end_p);

  void print_all();

  std::shared_ptr<bencode_value_base> get_value();

private:
  std::shared_ptr<bencode_value_base> value_;
};

#endif /* BENCODE_PARSER_H */
