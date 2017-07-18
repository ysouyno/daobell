#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <map>

class bencode_member
{
public:
  virtual void print_member() = 0;
};

class bencode_string : public bencode_member
{
public:
  bencode_string(const std::string &value) :
    value_(value)
  {
  }

  void print_member()
  {
    std::cout << value_ << std::endl;
  }

private:
  std::string value_;
};

class bencode_number : public bencode_member
{
public:
  bencode_number(long long value) :
    value_(value)
  {
  }

  void print_member()
  {
    std::cout << value_ << std::endl;
  }

private:
  long long value_;
};

class bencode_list : public bencode_member
{
public:
  void print_member()
  {
    for (std::vector<std::shared_ptr<bencode_member> >::iterator it = value_.begin();
         it != value_.end(); ++it) {
      (*it)->print_member();
    }
  }

  void insert_to_list(std::shared_ptr<bencode_member> value)
  {
    value_.push_back(value);
  }

private:
  std::vector<std::shared_ptr<bencode_member> > value_;
};

class bencode_dictionary : public bencode_member
{
public:
  void print_member()
  {
    for (std::multimap<std::shared_ptr<bencode_member>, std::shared_ptr<bencode_member> >::iterator it = value_.begin();
         it != value_.end(); ++it) {
      it->first.get()->print_member();
      it->second.get()->print_member();
    }
  }

  void insert_to_dictionary(std::shared_ptr<bencode_member> key, std::shared_ptr<bencode_member> value)
  {
    value_.insert(std::make_pair(key, value));
  }

private:
  std::multimap<std::shared_ptr<bencode_member>, std::shared_ptr<bencode_member> > value_;
};

class bencode_parser
{
public:
  bencode_parser() {}
  ~bencode_parser() {}

  bencode_parser(std::ifstream &ifs)
  {
    value_ = parse(ifs);
  }

  std::shared_ptr<bencode_member> parse(std::ifstream &ifs)
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

      return std::make_shared<bencode_number>(number);
    }
    case 'l': {
      bencode_list *bl = new bencode_list();

      while (ifs.peek() != 'e') {
        std::shared_ptr<bencode_member> value = parse(ifs);
        bl->insert_to_list(value);
      }

      ifs.get();
      return std::shared_ptr<bencode_member>(bl);
    }
    case 'd': {
      bencode_dictionary *bd = new bencode_dictionary();

      while (ifs.peek() != 'e') {
        std::shared_ptr<bencode_member> key = parse(ifs);
        std::shared_ptr<bencode_member> value = parse(ifs);
        bd->insert_to_dictionary(key, value);
      }

      ifs.get();
      return std::shared_ptr<bencode_member>(bd);
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

private:
  std::shared_ptr<bencode_member> value_;
};

#endif /* BENCODE_PARSER_H */
