#ifndef HTTP_HEADER_PARSE_H
#define HTTP_HEADER_PARSE_H

#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>
#include <map>
#include <sstream>

class http_header_parse
{
 public:
  http_header_parse(const char *str);
  ~http_header_parse() = default;
  void print();
  size_t get_file_size();
  size_t get_file_size_number_count();

 private:
  void parse();
  std::istringstream header_;
  std::map<std::string, std::string> header_map_;
};

#endif /* HTTP_HEADER_PARSE_H */
