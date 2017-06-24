#ifndef URL_PARSE_H
#define URL_PARSE_H

#include <string>
#include <boost/regex.hpp>
#include "log_wrapper.h"

struct url_parse
{
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

  std::string url_;
  std::string scheme_;
  std::string domain_;
  std::string port_;
  std::string path_;
  std::string query_;
  std::string fragment_;
};

url_parse::url_parse(const std::string &url) : url_(url)
{
  parse();
}

void url_parse::parse()
{
  if (url_.empty()) {
    log_e("url is null\n");
    return;
  }

  boost::regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
  boost::cmatch what;

  if (boost::regex_match(url_.c_str(), what,ex)) {
    scheme_ = std::string(what[1].first, what[1].second);
    domain_ = std::string(what[2].first, what[2].second);
    port_ = std::string(what[3].first, what[3].second);
    path_ = std::string(what[4].first, what[4].second);
    query_ = std::string(what[5].first, what[5].second);
    fragment_ = std::string(what[6].first, what[6].second);
  }
  else {
    log_e("url_parse()::parse input url not http\n");
  }
}

void url_parse::parse(const std::string &url)
{
  if (url.empty()) {
    log_e("url is null\n");
    return;
  }

  url_ = url;
  parse();
}

#endif /* URL_PARSE_H */
