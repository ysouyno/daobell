#ifndef FTP_URL_PARSER_H
#define FTP_URL_PARSER_H

#include <iostream>
#include <string>
#include <string.h>
#include "log_wrapper.h"

#define FTP_DEFAULT_PORT "21"

class ftp_url_parser
{
public:
  ftp_url_parser() = default;
  ~ftp_url_parser() = default;

  explicit ftp_url_parser(const std::string &url);

  void print_all();

private:
  int parse();

public:
  std::string url_;
  std::string user_;
  std::string password_;
  std::string domain_;
  std::string port_;
  std::string path_;
  std::string file_;
};

#endif /* FTP_URL_PARSER_H */
