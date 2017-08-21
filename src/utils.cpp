#include "utils.h"

namespace utils {

  std::string fmt(const char *fmt_template, ...)
  {
    va_list ap;
    va_start(ap, fmt_template);

    char buff[1024] = {0};
    if (vsnprintf(buff, sizeof(buff), fmt_template, ap) < 0) {
      buff[0] = '\0';
    }

    va_end(ap);

    return buff;
  }

  bool is_alpha(const char ch)
  {
    return
      (ch >= 'a' && ch <= 'z') ||
      (ch >= 'A' && ch <= 'Z');
  }

  bool is_digit(const char ch)
  {
    return
      (ch >= '0' && ch <= '9');
  }

  bool is_rfc3986_unreserved_chars(const char ch)
  {
    static const char unreserved[] = { '-', '.', '_', '~' };

    return
      is_alpha(ch) ||
      is_digit(ch) ||
      std::find(std::begin(unreserved), std::end(unreserved), ch) != std::end(unreserved);
  }

  std::string percent_encode(const unsigned char *target, size_t len)
  {
    std::string result;

    for (size_t i = 0; i < len; ++i) {
      if (is_rfc3986_unreserved_chars(target[i])) {
        result += target[i];
      }
      else {
        result.append(fmt("%%%02X", target[i]));
      }
    }

    return result;
  }

  std::string percent_encode(const std::string &target)
  {
    if (std::find_if_not(target.begin(), target.end(), is_rfc3986_unreserved_chars) == target.end()) {
      return target;
    }
    else {
      return percent_encode(reinterpret_cast<const unsigned char *>(target.c_str()), target.size());
    }
  }

  void hex_string_to_hex(const std::string &hex_str, std::string &hex)
  {
    if (!hex.empty()) {
      hex.clear();
    }

    for (size_t i = 0; i < hex_str.size(); i += 2) {
      int temp = 0;
      sscanf(hex_str.substr(i, 2).c_str(), "%2x", &temp);
      hex += temp;
    }
  }

}  // utils
