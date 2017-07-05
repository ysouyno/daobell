#ifndef HTTP_HEADER_PARSER_H
#define HTTP_HEADER_PARSER_H

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <map>
#include <sstream>

class http_header_parser
{
public:
  http_header_parser() {}
  http_header_parser(const char *str);
  ~http_header_parser() = default;

  void print();
  size_t get_file_size();
  size_t get_file_size_number_count();
  void parse(const char *str);

private:
  void parse();
  std::istringstream header_;
  std::map<std::string, std::string> header_map_;
};

#endif /* HTTP_HEADER_PARSER_H */
