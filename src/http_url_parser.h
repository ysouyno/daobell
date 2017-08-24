#ifndef HTTP_URL_PARSER_H
#define HTTP_URL_PARSER_H

#include <string>
#include <boost/regex.hpp>
#include "log_wrapper.h"

// why http_url_parser struct occur link error see below?
// "multiple definition of `http_url_parser::http_url_parser(std::string const&)'"
// change struct to class

class http_url_parser
{
public:
  http_url_parser() = default;
  ~http_url_parser() = default;

  explicit http_url_parser(const std::string &url);

  void parse();
  void parse(const std::string &url);

  bool valid()
  {
    return (!domain_.empty());
  }

  int operator()(const std::string &url)
  {
    if (url.empty()) {
      log_e("url is null\n");
      return -1;
    }

    url_ = url;
    parse();
    return 0;
  }

public:
  std::string url_;
  std::string scheme_;
  std::string domain_;
  std::string port_;
  std::string path_;
  std::string query_;
  std::string fragment_;
};

#endif /* HTTP_URL_PARSER_H */
