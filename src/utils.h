#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <cstdarg>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>

namespace utils {

  union sockaddr_union {
    sockaddr sa;
    sockaddr_storage storage;
    sockaddr_in6 in6;
    sockaddr_in in;
  };

  std::string fmt(const char *fmt_template, ...);

  bool is_alpha(const char ch);

  bool is_digit(const char ch);

  bool is_rfc3986_unreserved_chars(const char ch);

  std::string percent_encode(const unsigned char *target, size_t len);

  std::string percent_encode(const std::string &target);

  void hex_string_to_hex(const std::string &hex_str, std::string &hex);

  int inet_n_to_p(int af, const void *src, char *dst, socklen_t size);

  std::pair<std::string, uint16_t> unpack_compact(const unsigned char *compact);

}  // utils

#endif /* UTILS_H */
