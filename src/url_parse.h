#ifndef URL_PARSE_H
#define URL_PARSE_H

#include <string>
#include <boost/regex.hpp>
#include "log_wrapper.h"

// why url_parse struct occur link error see below?
// "multiple definition of `url_parse::url_parse(std::string const&)'"
// change struct to class

class url_parse
{
public:
  url_parse() {}
  ~url_parse() {}
  url_parse(const std::string &url);

  void parse();
  void parse(const std::string &url);

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

#endif /* URL_PARSE_H */
