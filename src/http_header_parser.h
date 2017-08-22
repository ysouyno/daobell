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
  http_header_parser() = default;
  ~http_header_parser() = default;

  explicit http_header_parser(const char *str);

  void parse(const char *str);
  size_t get_content_length();
  size_t get_content_length_number_count();
  const std::string &get_status();
  void print();

private:
  void parse();

  std::istringstream header_;
  std::map<std::string, std::string> header_map_;
};

#endif /* HTTP_HEADER_PARSER_H */
