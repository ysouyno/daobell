#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include "bencode_value_base.h"
#include "bencode_string.h"
#include "bencode_integer.h"
#include "bencode_list.h"
#include "bencode_dictionary.h"

class bencode_parser
{
public:
  bencode_parser() {}
  ~bencode_parser() {}

  bencode_parser(std::ifstream &ifs)
  {
    value_ = parse(ifs);
  }

  std::shared_ptr<bencode_value_base> parse(std::ifstream &ifs)
  {
    char ch = ifs.get();

    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      ifs.putback(ch);
      size_t length = 0;
      ifs >> length;
      ifs.get();

      std::vector<char> vec_temp(length);
      ifs.read(vec_temp.data(), vec_temp.size());
      std::string str_temp(vec_temp.begin(), vec_temp.end());

      return std::make_shared<bencode_string>(str_temp);
    }
    case 'i': {
      long long number;
      ifs >> number;
      ifs.get();

      return std::make_shared<bencode_integer>(number);
    }
    case 'l': {
      bencode_list *bl = new bencode_list();

      while (ifs.peek() != 'e') {
        std::shared_ptr<bencode_value_base> value = parse(ifs);
        bl->insert_to_list(value);
      }

      ifs.get();
      return std::shared_ptr<bencode_value_base>(bl);
    }
    case 'd': {
      bencode_dictionary *bd = new bencode_dictionary();

      while (ifs.peek() != 'e') {
        std::shared_ptr<bencode_value_base> key = parse(ifs);
        std::shared_ptr<bencode_value_base> value = parse(ifs);
        bd->insert_to_dictionary(key, value);
      }

      ifs.get();
      return std::shared_ptr<bencode_value_base>(bd);
    }
    default:
      // just return something
      return value_;
    }
  }

  void print_all()
  {
    value_.get()->print_member();
  }

  std::shared_ptr<bencode_value_base> get_value()
  {
    return value_;
  }

private:
  std::shared_ptr<bencode_value_base> value_;
};

#endif /* BENCODE_PARSER_H */
