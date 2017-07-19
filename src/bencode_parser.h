#ifndef BENCODE_PARSER_H
#define BENCODE_PARSER_H

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <map>

#define KEY_VALUE_DELIMITER "_KEY:VALUE_"
#define KEY_VALUE_END "\n"

class bencode_string;
class bencode_integer;
class bencode_list;
class bencode_dictionary;

class bencode_crawler
{
public:
  virtual void crawl(bencode_string *p) = 0;
  virtual void crawl(bencode_integer *p) = 0;
  virtual void crawl(bencode_list *p) = 0;
  virtual void crawl(bencode_dictionary *p) = 0;
};

class bencode_member
{
public:
  virtual void print_member() = 0;
  virtual void crawl(bencode_crawler *p) = 0;
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

  void crawl(bencode_crawler *p)
  {
    p->crawl(this);
  }

  const std::string &get_value() const
  {
    return value_;
  }

private:
  std::string value_;
};

class bencode_integer : public bencode_member
{
public:
  bencode_integer(long long value) :
    value_(value)
  {
  }

  void print_member()
  {
    std::cout << value_ << std::endl;
  }

  void crawl(bencode_crawler *p)
  {
    p->crawl(this);
  }

  long long get_value() const
  {
    return value_;
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

  void crawl(bencode_crawler *p)
  {
    p->crawl(this);
  }

  void insert_to_list(std::shared_ptr<bencode_member> value)
  {
    value_.push_back(value);
  }

  const std::vector<std::shared_ptr<bencode_member> > &get_value() const
  {
    return value_;
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

  void crawl(bencode_crawler *p)
  {
    p->crawl(this);
  }

  void insert_to_dictionary(std::shared_ptr<bencode_member> key, std::shared_ptr<bencode_member> value)
  {
    value_.insert(std::make_pair(key, value));
  }

  const std::multimap<std::shared_ptr<bencode_member>, std::shared_ptr<bencode_member> > &get_value() const
  {
    return value_;
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

      return std::make_shared<bencode_integer>(number);
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

  std::shared_ptr<bencode_member> get_value()
  {
    return value_;
  }

private:
  std::shared_ptr<bencode_member> value_;
};

class bencode_collector : public bencode_crawler
{
public:
  bencode_collector(std::shared_ptr<bencode_member> sp_bencode_parser) :
    sp_bencode_parser_(sp_bencode_parser)
  {
  }

  virtual void crawl(bencode_string *p)
  {
    bencode_all_string_ += p->get_value();
  }

  virtual void crawl(bencode_integer *p)
  {
    bencode_all_string_ += std::to_string(p->get_value());
  }

  virtual void crawl(bencode_list *p)
  {
    std::vector<std::shared_ptr<bencode_member> > value = p->get_value();

    for (std::vector<std::shared_ptr<bencode_member> >::iterator it = value.begin();
         it != value.end(); ++it) {
      bencode_all_string_ += KEY_VALUE_DELIMITER;
      (*it)->crawl(this);
      bencode_all_string_ += KEY_VALUE_END;
    }
  }

  virtual void crawl(bencode_dictionary *p)
  {
    std::multimap<std::shared_ptr<bencode_member>, std::shared_ptr<bencode_member> > value = p->get_value();

    for (std::multimap<std::shared_ptr<bencode_member>, std::shared_ptr<bencode_member> >::iterator it = value.begin();
         it != value.end(); ++it) {
      it->first.get()->crawl(this);
      bencode_all_string_ += KEY_VALUE_DELIMITER;
      it->second.get()->crawl(this);
      bencode_all_string_ += KEY_VALUE_END;
    }
  }

  void print_all()
  {
    crawl_all();
    std::cout << bencode_all_string_ << std::endl;
  }

  void crawl_all()
  {
    sp_bencode_parser_->crawl(this);
  }

  /* FIXME: [ ] parse list type well */
  /* FIXME: [ ] do not use strtok to find delimiter */
  void save_to_multimap()
  {
    std::string str = bencode_all_string_;

    char *p = strtok(const_cast<char *>(str.c_str()), KEY_VALUE_END);
    while (p) {
      std::string temp(p);

      size_t pos1 = temp.find(KEY_VALUE_DELIMITER);
      size_t pos2 = temp.rfind(KEY_VALUE_DELIMITER);

      if (pos1 == pos2) {
        std::string key = temp.substr(0, pos1);
        std::string value = temp.substr(pos1 + strlen(KEY_VALUE_DELIMITER),
                                        temp.size() - pos1 - strlen(KEY_VALUE_DELIMITER));
        multimap_dictionary_.insert(std::make_pair(key, value));
      }
      else {
        std::string key = temp.substr(0, pos1);
        std::string value = "";
        multimap_dictionary_.insert(std::make_pair(key, value));

        key = temp.substr(pos1 + strlen(KEY_VALUE_DELIMITER),
                          pos2 - pos1 - strlen(KEY_VALUE_DELIMITER));
        value = temp.substr(pos2 + strlen(KEY_VALUE_DELIMITER),
                            temp.size() - pos2 - strlen(KEY_VALUE_DELIMITER));
        multimap_dictionary_.insert(std::make_pair(key, value));
      }

      p = strtok(NULL, KEY_VALUE_END);
    }
  }

  void print_multimap()
  {
    crawl_all();
    save_to_multimap();
    for (std::multimap<std::string, std::string>::iterator it = multimap_dictionary_.begin();
         it != multimap_dictionary_.end(); ++it) {
      std::cout << it->first << " : " << it->second << std::endl;
    }
  }

private:
  std::string bencode_all_string_;
  std::shared_ptr<bencode_member> sp_bencode_parser_;
  std::multimap<std::string, std::string> multimap_dictionary_;
};

#endif /* BENCODE_PARSER_H */
