#include "http_header_parser.h"

http_header_parser::http_header_parser(const char *str)
: header_(str)
{
  parse();
}

void http_header_parser::parse()
{
  std::string header;
  std::string::size_type index;

  while (std::getline(header_, header) && "\r" != header) {
    index = header.find("HTTP/1.");
    if (std::string::npos != index) {
      header_map_.insert(std::make_pair("Status", header.substr(header.find_first_of(' ') + 1, header.find_last_of(' ') - 1)));
    }

    index = header.find(':', 0);
    if (std::string::npos != index) {
      header_map_.insert(std::make_pair(boost::algorithm::trim_copy(header.substr(0, index)), boost::algorithm::trim_copy(header.substr(index + 1))));
    }
  }
}

void http_header_parser::parse(const char *str)
{
  std::string str_tmp(str);
  header_.str(str_tmp);

  parse();
}

size_t http_header_parser::get_content_length()
{
  size_t file_size = 0;
  for (std::map<std::string, std::string>::iterator it = header_map_.begin(); it != header_map_.end(); ++it) {
    if (std::string::npos != it->first.find("Content-Length")) {
      std::istringstream(it->second) >> file_size;
    }
  }

  return file_size;
}

size_t http_header_parser::get_content_length_number_count()
{
  size_t count = 0;
  size_t content_length = get_content_length();
  while (content_length > 0) {
    count++;
    content_length /= 10;
  }

  return count;
}

const std::string &http_header_parser::get_status()
{
  return header_map_.at("Status");
}

void http_header_parser::print()
{
  for (std::map<std::string, std::string>::iterator it = header_map_.begin(); it != header_map_.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}
