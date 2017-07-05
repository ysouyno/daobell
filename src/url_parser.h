#ifndef URL_PARSER_H
#define URL_PARSER_H

#include <string>
#include <boost/regex.hpp>
#include "log_wrapper.h"

// why url_parser struct occur link error see below?
// "multiple definition of `url_parser::url_parser(std::string const&)'"
// change struct to class

class url_parser
{
public:
  url_parser() {}
  ~url_parser() {}
  url_parser(const std::string &url);

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

#endif /* URL_PARSER_H */
