#include "http_header_parse.h"

http_header_parse::http_header_parse(const char *str)
: header_(str)
{
  parse();
}

void http_header_parse::parse()
{
  std::string header;
  std::string::size_type index;

  while (std::getline(header_, header) && "\r" != header) {
    index = header.find(':', 0);
    if (std::string::npos != index) {
      header_map_.insert(std::make_pair(boost::algorithm::trim_copy(header.substr(0, index)), boost::algorithm::trim_copy(header.substr(index + 1))));
    }
  }
}

void http_header_parse::print()
{
  for (std::map<std::string, std::string>::iterator it = header_map_.begin(); it != header_map_.end(); ++it) {
    std::cout << it->first << ": " << it->second << std::endl;
  }
}

size_t http_header_parse::get_file_size()
{
  size_t file_size = 0;
  for (std::map<std::string, std::string>::iterator it = header_map_.begin(); it != header_map_.end(); ++it) {
    if (std::string::npos != it->first.find("Content-Length")) {
      std::istringstream(it->second) >> file_size;
    }
  }

  return file_size;
}

size_t http_header_parse::get_file_size_number_count()
{
  size_t count = 0;
  size_t file_size = get_file_size();
  while (file_size > 0) {
    count++;
    file_size /= 10;
  }

  return count;
}
