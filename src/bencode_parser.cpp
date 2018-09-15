#include "bencode_parser.h"

bencode_parser::bencode_parser(std::ifstream &ifs)
{
  value_ = parse(ifs);
}

bencode_parser::bencode_parser(const char *p)
{
  const char *end_p = NULL;
  value_ = parse(p, &end_p);
}

std::shared_ptr<bencode_value_base> bencode_parser::parse(std::ifstream &ifs)
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
    bencode_dict *bd = new bencode_dict();

    while (ifs.peek() != 'e') {
      // key is a string always
      std::shared_ptr<bencode_value_base> sp_bvb_key = parse(ifs);
      std::string key = dynamic_cast<bencode_string *>(sp_bvb_key.get())->get_value();
      std::shared_ptr<bencode_value_base> value = parse(ifs);
      bd->insert_to_dict(key, value);
    }

    ifs.get();
    return std::shared_ptr<bencode_value_base>(bd);
  }
  default:
    // just return something
    return value_;
  }
}

// reference from the BitFiend project (https://github.com/eduard-permyakov/BitFiend.git)
std::shared_ptr<bencode_value_base> bencode_parser::parse(const char *p, const char **end_p)
{
  char ch = *p;

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
    *end_p = p;
    size_t length = strtol(p, const_cast<char **>(end_p), 10);
    (*end_p)++;

    std::vector<char> vec_temp(*end_p, *end_p + length);
    std::string str_temp(vec_temp.begin(), vec_temp.end());
    *end_p += length;

    return std::make_shared<bencode_string>(str_temp);
  }
  case 'i': {
    *end_p = p;
    p++;
    size_t number = strtol(p, const_cast<char **>(end_p), 10);
    (*end_p)++;

    return std::make_shared<bencode_integer>(number);
  }
  case 'l': {
    *end_p = p + 1;
    bencode_list *bl = new bencode_list();

    while (**end_p != 'e') {
      p = *end_p;
      std::shared_ptr<bencode_value_base> value = parse(p, end_p);
      bl->insert_to_list(value);
    }

    (*end_p)++;
    return std::shared_ptr<bencode_value_base>(bl);
  }
  case 'd': {
    *end_p = p + 1;
    bencode_dict *bd = new bencode_dict();

    while (**end_p != 'e') {
      // key is a string always
      p = *end_p;
      std::shared_ptr<bencode_value_base> sp_bvb_key = parse(p, end_p);
      std::string key = dynamic_cast<bencode_string *>(sp_bvb_key.get())->get_value();

      p = *end_p;
      std::shared_ptr<bencode_value_base> value = parse(p, end_p);
      bd->insert_to_dict(key, value);
    }

    (*end_p)++;
    return std::shared_ptr<bencode_value_base>(bd);
  }
  default:
    // just return something
    return value_;
  }
}

void bencode_parser::print_all()
{
  value_.get()->print_member();
}

std::shared_ptr<bencode_value_base> bencode_parser::get_value()
{
  return value_;
}
