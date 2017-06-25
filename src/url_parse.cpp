#include "url_parse.h"

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
    log_w("url_parse()::parse input url not http\n");
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
