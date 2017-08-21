#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <cstdarg>

namespace utils {

  std::string fmt(const char *fmt_template, ...);

  bool is_alpha(const char ch);

  bool is_digit(const char ch);

  bool is_rfc3986_unreserved_chars(const char ch);

  std::string percent_encode(const unsigned char *target, size_t len);

  std::string percent_encode(const std::string &target);

  void hex_string_to_hex(const std::string &hex_str, std::string &hex);

}  // utils

#endif /* UTILS_H */
